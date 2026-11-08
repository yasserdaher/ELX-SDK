/* Basic ELBC reader fuzz harness: feeds mutated/corrupt inputs to elx_module_load.
 * Goal: no crash, no UB observed under ASan. Not a coverage-guided fuzzer. */
#include "elx_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

static unsigned x = 0x12345678u;
static unsigned rnd(void){ x = x*1664525u+1013904223u; return x; }

static void try_load(const char *path){
    elx_module_t m; memset(&m,0,sizeof(m));
    if (elx_module_load(path,&m)) elx_module_destroy(&m);
}
static void write_tmp(const char *p,const void *d,size_t n){ FILE*f=fopen(p,"wb"); if(f){fwrite(d,1,n,f); fclose(f);} }

int main(void){
    const char *fix = "examples/hello/main.elbc";
    FILE *f=fopen(fix,"rb"); if(!f){ fprintf(stderr,"skip: no fixture\n"); return 0; }
    fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
    unsigned char *base=malloc((size_t)sz); fread(base,1,(size_t)sz,f); fclose(f);
    char path[256]; snprintf(path,sizeof(path),"/tmp/fuzz_elbc_%d.elbc",(int)getpid());
    unsetenv("ELX_ELBC_ALLOW_BAD_CRC");
    for(int i=0;i<2000;i++){
        size_t n = (size_t)(rnd() % (unsigned)(sz*2 + 64));
        if(n==0) n=1;
        unsigned char *buf=calloc(1,n);
        size_t copy = n < (size_t)sz ? n : (size_t)sz;
        memcpy(buf,base,copy);
        unsigned muts = 1 + (rnd()%32);
        for(unsigned m=0;m<muts;m++) if(n) buf[rnd()%n] ^= (unsigned char)(rnd()&0xFF);
        /* structured: corrupt lengths/counts */
        if(n>40 && (rnd()%4)==0){ uint32_t v=rnd(); memcpy(buf+20+(rnd()%5)*4,&v,4); }
        if(n>44 && (rnd()%4)==0){ uint32_t v=rnd()&0x7FFFFFFFu; memcpy(buf+28,&v,4); }
        write_tmp(path,buf,n);
        try_load(path);
        free(buf);
    }
    /* edge sizes */
    for(size_t n=0;n<80;n++){ write_tmp(path,base,n<(size_t)sz?n:(size_t)sz); try_load(path); }
    unlink(path); free(base);
    printf("fuzz_elbc_reader: 2000 mutations + edge cases completed (no crash observed)\n");
    return 0;
}
