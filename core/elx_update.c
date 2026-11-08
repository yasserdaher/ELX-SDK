/*
 * ELX Update System — implementation (GitHub Releases, HTTPS via curl, SHA-256, safe install)
 * Safety: no curl|bash, no shell with release data, HTTPS only, checksum required, atomic swap.
 */
#define _POSIX_C_SOURCE 200809L
#include "elx_update.h"
#include "elx_core.h"
#include "elx_sdk_security.h"
#include "elx_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#if defined(__unix__) || defined(__APPLE__)
#include <spawn.h>
extern char **environ;
#endif

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define ELX_PATH_SEP '\\'
#else
#define ELX_PATH_SEP '/'
#endif

static const char *g_test_api_json; static size_t g_test_api_len;
static const char *g_test_asset_path;
static const char *g_test_checksum_path;

void elx_update_set_test_api_json(const char *json, size_t len){ g_test_api_json=json; g_test_api_len=len; }
void elx_update_set_test_asset_path(const char *path){ g_test_asset_path=path; }
void elx_update_set_test_checksum_path(const char *path){ g_test_checksum_path=path; }
void elx_update_clear_test_hooks(void){ g_test_api_json=NULL; g_test_api_len=0; g_test_asset_path=NULL; g_test_checksum_path=NULL; }

const char *elx_update_status_string(elx_update_status_t st){
    switch(st){
        case ELX_UPDATE_OK: return "ok";
        case ELX_UPDATE_UP_TO_DATE: return "up-to-date";
        case ELX_UPDATE_AVAILABLE: return "update-available";
        case ELX_UPDATE_ERR_NETWORK: return "network error";
        case ELX_UPDATE_ERR_API: return "GitHub API error";
        case ELX_UPDATE_ERR_RATE_LIMIT: return "GitHub API rate limit";
        case ELX_UPDATE_ERR_NO_RELEASE: return "no suitable release found";
        case ELX_UPDATE_ERR_UNSUPPORTED_PLATFORM: return "unsupported platform/architecture";
        case ELX_UPDATE_ERR_NO_ASSET: return "no matching release asset for this platform";
        case ELX_UPDATE_ERR_NO_CHECKSUM: return "no trusted SHA-256 checksum found; refusing automatic install";
        case ELX_UPDATE_ERR_DOWNLOAD: return "download failed";
        case ELX_UPDATE_ERR_CHECKSUM: return "SHA-256 verification failed";
        case ELX_UPDATE_ERR_INSTALL: return "installation failed";
        case ELX_UPDATE_ERR_PERMISSIONS: return "insufficient permissions to install";
        case ELX_UPDATE_ERR_NOT_INSTALLED: return "not an installed binary (running from build/source); refusing to modify source tree";
        case ELX_UPDATE_ERR_VERSION: return "version parse/comparison error";
        case ELX_UPDATE_ERR_UNSAFE: return "unsafe update refused";
        default: return "unknown";
    }
}

static void trim(char *s){ if(!s) return; char *p=s; while(*p && isspace((unsigned char)*p)) p++; if(p!=s) memmove(s,p,strlen(p)+1); size_t n=strlen(s); while(n && isspace((unsigned char)s[n-1])) s[--n]=0; }

bool elx_update_parse_github_remote(const char *origin_url, char *owner, size_t owner_sz, char *repo, size_t repo_sz){
    if(!origin_url||!owner||!repo||owner_sz<2||repo_sz<2) return false;
    owner[0]=repo[0]=0;
    const char *p = strstr(origin_url, "github.com");
    if(!p) return false;
    p = strstr(origin_url, "github.com");
    p += strlen("github.com");
    if(*p=='/'||*p==':') p++;
    /* now owner/repo... */
    const char *sl = strchr(p,'/');
    if(!sl) return false;
    size_t ol = (size_t)(sl-p);
    if(ol==0 || ol>=owner_sz) return false;
    memcpy(owner,p,ol); owner[ol]=0;
    p = sl+1;
    const char *end = p + strcspn(p, "/?#");
    size_t rl = (size_t)(end-p);
    if(rl>4 && strcmp(end-4, ".git")==0) rl-=4;
    if(rl==0 || rl>=repo_sz) return false;
    memcpy(repo,p,rl); repo[rl]=0;
    /* basic safety: allow alnum, -, _, . */
    for(size_t i=0;owner[i];++i){ unsigned char c=(unsigned char)owner[i]; if(!isalnum(c)&&c!='-'&&c!='_'&&c!='.') return false; }
    for(size_t i=0;repo[i];++i){ unsigned char c=(unsigned char)repo[i]; if(!isalnum(c)&&c!='-'&&c!='_'&&c!='.') return false; }
    return owner[0] && repo[0];
}

static bool read_cmd_stdout(const char *cmd, char *out, size_t out_sz){
    if(!cmd||!out||out_sz==0) return false; out[0]=0;
    FILE *fp = popen(cmd, "r"); /* fixed command string only; never includes release data */
    if(!fp) return false;
    size_t n = fread(out,1,out_sz-1,fp); out[n]=0; pclose(fp); trim(out);
    return out[0]!=0;
}

bool elx_update_detect_repo(char *owner, size_t owner_sz, char *repo, size_t repo_sz){
    const char *env = getenv("ELX_UPDATE_REPO");
    if(env && env[0]){
        char tmp[256]; snprintf(tmp,sizeof(tmp),"%s",env);
        char *sl=strchr(tmp,'/'); if(!sl) return false; *sl=0;
        snprintf(owner,owner_sz,"%s",tmp); snprintf(repo,repo_sz,"%s",sl+1);
        return owner[0]&&repo[0];
    }
    char origin[512];
    if(!read_cmd_stdout("git remote get-url origin 2>/dev/null", origin, sizeof(origin))) return false;
    return elx_update_parse_github_remote(origin, owner, owner_sz, repo, repo_sz);
}

int elx_update_compare_versions(const char *a, const char *b){
    /* returns -1 if a<b, 0 eq, 1 if a>b; supports 1.2.3 and 1.2.3-phase4 */
    if(!a||!b) return 0;
    int am[3]={0,0,0}, bm[3]={0,0,0}, ap=0,bp=0;
    sscanf(a, "%d.%d.%d", &am[0],&am[1],&am[2]);
    sscanf(b, "%d.%d.%d", &bm[0],&bm[1],&bm[2]);
    const char *pa=strstr(a,"phase"); if(pa) ap=atoi(pa+5);
    const char *pb=strstr(b,"phase"); if(pb) bp=atoi(pb+5);
    for(int i=0;i<3;++i){ if(am[i]<bm[i]) return -1; if(am[i]>bm[i]) return 1; }
    if(ap<bp) return -1; if(ap>bp) return 1; return 0;
}

void elx_update_platform_id(char *out, size_t out_sz){
#if defined(_WIN32)
    const char *os="windows";
#elif defined(__APPLE__)
    const char *os="macos";
#else
    const char *os="linux";
#endif
#if defined(__x86_64__)||defined(_M_X64)||defined(__amd64__)
    const char *arch="x86_64";
#elif defined(__aarch64__)||defined(_M_ARM64)
    const char *arch="arm64";
#else
    const char *arch=ELX_ARCH_NAME;
#endif
    snprintf(out,out_sz,"%s-%s",os,arch);
}

/* --- minimal JSON helpers (no external deps) --- */
static bool json_get_bool(const char *json, const char *key, bool *out){
    char pat[128]; snprintf(pat,sizeof(pat),"\"%s\"",key);
    const char *p=strstr(json,pat); if(!p) return false; p=strchr(p,':'); if(!p) return false; p++;
    while(*p && isspace((unsigned char)*p)) p++;
    if(strncmp(p,"true",4)==0){ *out=true; return true; }
    if(strncmp(p,"false",5)==0){ *out=false; return true; }
    return false;
}
static bool json_get_string(const char *json, const char *key, char *out, size_t out_sz){
    char pat[128]; snprintf(pat,sizeof(pat),"\"%s\"",key);
    const char *p=strstr(json,pat); if(!p) return false; p=strchr(p,':'); if(!p) return false; p=strchr(p,'"'); if(!p) return false; p++;
    size_t i=0; while(*p && *p!='"' && i+1<out_sz){ if(*p=='\\'&&p[1]){ p++; if(*p=='n') out[i++]='\n'; else out[i++]=*p; p++; } else out[i++]=*p++; }
    out[i]=0; return true;
}

static char *read_file_all(const char *path, size_t *out_len){
    FILE *f=fopen(path,"rb"); if(!f) return NULL; fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
    if(sz<0){ fclose(f); return NULL; } char *b=malloc((size_t)sz+1); if(!b){ fclose(f); return NULL; }
    size_t n=fread(b,1,(size_t)sz,f); b[n]=0; fclose(f); if(out_len) *out_len=n; return b;
}

static bool https_url(const char *url){ return url && strncmp(url,"https://",8)==0; }


#if defined(__unix__) || defined(__APPLE__)
static int run_curl_argv(char *const argv[]){
    pid_t pid = 0;
    int rc = posix_spawnp(&pid, "curl", NULL, NULL, argv, environ);
    if (rc != 0) return -1;
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return -1;
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
}
#endif

static elx_update_status_t download_https(const char *url, const char *dest){
    if(!https_url(url)) return ELX_UPDATE_ERR_UNSAFE;
    /* reject control chars / whitespace surprises in URL/dest passed to argv */
    for(const char *p=url; *p; ++p){ unsigned char c=(unsigned char)*p; if(c<0x21||c>0x7e) return ELX_UPDATE_ERR_UNSAFE; }
    if(strchr(dest,'\'')) return ELX_UPDATE_ERR_UNSAFE;
#if defined(__unix__) || defined(__APPLE__)
    char *const argv[] = { "curl", "-fsSL", "--proto", "=https", "--tlsv1.2", "-o", (char*)dest, (char*)url, NULL };
    int rc = run_curl_argv(argv);
#else
    char cmd[1024];
    snprintf(cmd,sizeof(cmd),"curl -fsSL --proto \"=https\" --tlsv1.2 -o \"%s\" \"%s\"", dest, url);
    if(strchr(url,'"')||strchr(dest,'"')) return ELX_UPDATE_ERR_UNSAFE;
    int rc = system(cmd);
#endif
    if(rc!=0) return ELX_UPDATE_ERR_DOWNLOAD;
    struct stat st; if(stat(dest,&st)!=0 || st.st_size<=0) return ELX_UPDATE_ERR_DOWNLOAD;
    return ELX_UPDATE_OK;
}

static bool sha256_file_hex(const char *path, char out_hex[65]){
    unsigned char h[32];
    if(elx_sec_hash_file_sha256(path,h,sizeof(h)) != ELX_SEC_OK) return false;
    static const char *hex="0123456789abcdef";
    for(int i=0;i<32;++i){ out_hex[i*2]=hex[(h[i]>>4)&0xF]; out_hex[i*2+1]=hex[h[i]&0xF]; }
    out_hex[64]=0; return true;
}

static bool find_asset_for_platform(const char *json, const char *platform, elx_update_info_t *info){
    /* scan assets array: find "name" containing platform and (.zip|.tar.gz|.exe) and "browser_download_url" (https) */
    const char *assets = strstr(json, "\"assets\"");
    if(!assets) return false;
    const char *p = assets;
    while((p=strstr(p,"\"browser_download_url\""))!=NULL){
        char url[512]={0}, name[256]={0};
        /* get url */
        const char *u=strchr(p,':'); if(!u) break; u=strchr(u,'"'); if(!u) break; u++;
        size_t i=0; while(*u && *u!='"' && i+1<sizeof(url)){ if(*u=='\\'&&u[1]) u++; url[i++]=*u++; } url[i]=0;
        /* find nearest preceding name in this object (heuristic) */
        const char *obj = p; while(obj>assets && *obj!='{') obj--;
        const char *np = strstr(obj,"\"name\"");
        if(np && np < p){ const char *n=strchr(np,':'); if(n){ n=strchr(n,'"'); if(n){ n++; i=0; while(*n&&*n!='"'&&i+1<sizeof(name)){ if(*n=='\\'&&n[1]) n++; name[i++]=*n++; } name[i]=0; } } }
        bool plat = (strstr(name,platform)!=NULL) || (strstr(url,platform)!=NULL);
        bool pkg = strstr(name,".zip")||strstr(name,".tar.gz")||strstr(name,".exe")||strstr(url,".zip")||strstr(url,".tar.gz");
        bool nocs = strstr(name,".sha256")==NULL && strstr(name,"checksum")==NULL;
        if(plat && pkg && nocs && https_url(url)){
            snprintf(info->asset_name,sizeof(info->asset_name),"%s",name);
            snprintf(info->asset_url,sizeof(info->asset_url),"%s",url);
            /* checksum asset: look for name.sha256 or SHA256SUMS in assets */
            return true;
        }
        p++;
    }
    return false;
}

static bool find_checksum_url(const char *json, const char *asset_name, char *out, size_t out_sz){
    /* prefer <asset_name>.sha256 */
    char want[300]; snprintf(want,sizeof(want),"%s.sha256",asset_name);
    const char *p = json;
    while((p=strstr(p,"\"browser_download_url\""))!=NULL){
        char url[512]={0}, name[256]={0};
        const char *u=strchr(p,':'); if(!u) break; u=strchr(u,'"'); if(!u) break; u++;
        size_t i=0; while(*u&&*u!='"'&&i+1<sizeof(url)){ if(*u=='\\'&&u[1]) u++; url[i++]=*u++; } url[i]=0;
        const char *obj=p; while(obj>json && *obj!='{') obj--;
        const char *np=strstr(obj,"\"name\""); if(np&&np<p){ const char *n=strchr(np,':'); if(n){n=strchr(n,'"'); if(n){n++; i=0; while(*n&&*n!='"'&&i+1<sizeof(name)){ if(*n=='\\'&&n[1]) n++; name[i++]=*n++; } name[i]=0;}}}
        if((strcmp(name,want)==0 || strstr(name,"SHA256")!=NULL || strstr(name,"sha256")!=NULL) && https_url(url)){
            snprintf(out,out_sz,"%s",url); return true;
        }
        p++;
    }
    return false;
}

/* Strict SHA-256 manifest parsing: exact asset-name binding, no arbitrary fallback. */
bool elx_update_parse_sha256_manifest(const char *checksum_text, const char *asset_name,
                                      char out_hex[65]) {
    if (!checksum_text || !asset_name || !asset_name[0] || !out_hex) return false;
    out_hex[0] = 0;
    char matched[65] = {0};
    int matches = 0;
    const char *p = checksum_text;
    while (p && *p) {
        const char *e = strchr(p, '\n');
        if (!e) e = p + strlen(p);
        /* copy/trim a complete logical line */
        size_t raw = (size_t)(e - p);
        char line[1024];
        if (raw >= sizeof(line)) raw = sizeof(line) - 1;
        memcpy(line, p, raw); line[raw] = 0;
        /* strip trailing CR/whitespace */
        size_t ln = strlen(line);
        while (ln && (line[ln-1]=='\r' || isspace((unsigned char)line[ln-1]))) line[--ln] = 0;
        /* skip empty/comment lines */
        const char *q = line;
        while (*q && isspace((unsigned char)*q)) q++;
        if (*q && *q != '#') {
            /* parse 64 hex chars */
            char hex[65] = {0}; int n = 0;
            while (q[n] && isxdigit((unsigned char)q[n]) && n < 64) { hex[n] = (char)tolower((unsigned char)q[n]); n++; }
            hex[n] = 0;
            if (n != 64) return false; /* malformed checksum token */
            const char *rest = q + 64;
            if (*rest && !isspace((unsigned char)*rest)) return false; /* malformed: junk after hash */
            while (*rest && isspace((unsigned char)*rest)) rest++;
            /* documented formats: "<hex>  <filename>" and "<hex> *<filename>" and "<hex> <filename>" */
            if (*rest == '*') rest++; /* binary mode marker (BSD/coreutils compat) */
            /* filename is the remaining field, trimmed (no spaces in ELX asset names) */
            while (*rest && isspace((unsigned char)*rest)) rest++;
            if (*rest == 0) return false; /* hash with no filename cannot be safely bound */
            char fname[512]; size_t fi = 0;
            while (rest[fi] && !isspace((unsigned char)rest[fi]) && fi + 1 < sizeof(fname)) { fname[fi] = rest[fi]; fi++; }
            fname[fi] = 0;
            /* reject extra unexpected filename fields (ambiguous line) */
            const char *tail = rest + fi;
            while (*tail && isspace((unsigned char)*tail)) tail++;
            if (*tail != 0) return false;
            if (strcmp(fname, asset_name) == 0) {
                matches++;
                if (matches > 1) return false; /* duplicate/ambiguous match */
                memcpy(matched, hex, 65);
            }
        }
        p = (*e) ? e + 1 : NULL;
    }
    if (matches != 1) return false; /* missing requested asset */
    memcpy(out_hex, matched, 65);
    return true;
}

static bool parse_expected_sha256(const char *checksum_text, const char *asset_name, char out_hex[65]){
    return elx_update_parse_sha256_manifest(checksum_text, asset_name, out_hex);
}

static bool is_build_tree_binary(const char *exe_path){
    if(!exe_path) return true;
    /* if path contains /build/ or ends up inside repo build dirs, treat as non-installed */
    return strstr(exe_path, "/build/")!=NULL || strstr(exe_path, "\\build\\")!=NULL;
}

static void get_self_path(char *out, size_t out_sz){
    out[0]=0;
#if defined(_WIN32)
    GetModuleFileNameA(NULL,out,(DWORD)out_sz);
#else
    ssize_t n = readlink("/proc/self/exe", out, out_sz-1);
    if(n>0) out[n]=0; else {
        /* fallback */
        snprintf(out,out_sz,"/proc/self/exe");
    }
#endif
}

static elx_update_status_t fetch_latest_release_json(const char *owner, const char *repo, char **out_json){
    *out_json=NULL;
    if(g_test_api_json){ *out_json=strdup(g_test_api_json); return *out_json?ELX_UPDATE_OK:ELX_UPDATE_ERR_API; }
    char url[512];
    snprintf(url,sizeof(url),"https://api.github.com/repos/%s/%s/releases/latest", owner, repo);
    if(!https_url(url)) return ELX_UPDATE_ERR_UNSAFE;
    char tmp[256]; snprintf(tmp,sizeof(tmp),"/tmp/elx_update_api_%d.json",(int)getpid());
    char cmd[1024];
    /* fixed argv, owner/repo validated to safe chars; no shell */
    if(strchr(owner,'\'')||strchr(repo,'\'')||strchr(url,'\'')) return ELX_UPDATE_ERR_UNSAFE;
#if defined(__unix__) || defined(__APPLE__)
    char *const argv[] = { "curl", "-fsSL", "--proto", "=https", "--tlsv1.2",
                           "-H", "Accept: application/vnd.github+json",
                           "-H", "User-Agent: ELX-SDK-Updater",
                           "-o", tmp, (char*)url, NULL };
    int rc = run_curl_argv(argv);
#else
    char cmd[1024];
    snprintf(cmd,sizeof(cmd),"curl -fsSL --proto \"=https\" --tlsv1.2 -H \"Accept: application/vnd.github+json\" -H \"User-Agent: ELX-SDK-Updater\" -o \"%s\" \"%s\"", tmp, url);
    int rc = system(cmd);
#endif
    if(rc!=0){ unlink(tmp); return ELX_UPDATE_ERR_NETWORK; }
    size_t len=0; char *j=read_file_all(tmp,&len); unlink(tmp);
    if(!j || len==0){ free(j); return ELX_UPDATE_ERR_API; }
    if(strstr(j,"rate limit") || strstr(j,"API rate limit")){ free(j); return ELX_UPDATE_ERR_RATE_LIMIT; }
    if(strstr(j,"\"message\"") && strstr(j,"Not Found")){ free(j); return ELX_UPDATE_ERR_NO_RELEASE; }
    *out_json=j; return ELX_UPDATE_OK;
}

elx_update_status_t elx_update_run(const elx_update_options_t *opts, elx_update_info_t *out_info){
    elx_update_options_t o = opts?*opts:(elx_update_options_t){0};
    elx_update_info_t info; memset(&info,0,sizeof(info));
    snprintf(info.current,sizeof(info.current),"%s", elx_sdk_version()?elx_sdk_version():"0.0.0");
    char platform[64]; elx_update_platform_id(platform, sizeof(platform));
    info.notes[0]=0;

    char owner[128]={0}, repo[128]={0};
    if(!elx_update_detect_repo(owner,sizeof(owner),repo,sizeof(repo))){
        return ELX_UPDATE_ERR_API; /* cannot determine official repo safely */
    }

    char *json=NULL;
    elx_update_status_t st = fetch_latest_release_json(owner,repo,&json);
    if(st!=ELX_UPDATE_OK) return st;

    bool draft=false, prerelease=false;
    json_get_bool(json,"draft",&draft);
    json_get_bool(json,"prerelease",&prerelease);
    if(draft || prerelease){ free(json); return ELX_UPDATE_ERR_NO_RELEASE; }
    json_get_string(json,"tag_name",info.tag,sizeof(info.tag));
    json_get_string(json,"html_url",info.release_url,sizeof(info.release_url));
    json_get_string(json,"published_at",info.published_at,sizeof(info.published_at));
    json_get_string(json,"body",info.notes,sizeof(info.notes));
    info.is_prerelease=prerelease;
    /* normalize latest: strip leading v */
    const char *lv = info.tag; if(lv[0]=='v'||lv[0]=='V') lv++;
    snprintf(info.latest,sizeof(info.latest),"%s", lv[0]?lv:info.tag);
    if(info.latest[0]==0) snprintf(info.latest,sizeof(info.latest),"%s", info.current);

    if(elx_update_compare_versions(info.latest, info.current) > 0) info.has_update=true;
    else info.has_update=false;

    if(o.target_version && o.target_version[0]){
        /* specific version requested: for safety, only allow if it matches latest stable discovered
         * (full release-by-tag fetch can be added; avoid guessing endpoints). */
        const char *tv=o.target_version; if(tv[0]=='v'||tv[0]=='V') tv++;
        if(elx_update_compare_versions(tv, info.latest)!=0 && elx_update_compare_versions(tv, info.current)!=0){
            /* not resolvable via latest; refuse rather than guess */
            free(json); return ELX_UPDATE_ERR_NO_RELEASE;
        }
        snprintf(info.latest,sizeof(info.latest),"%s", tv);
        info.has_update = (elx_update_compare_versions(info.latest, info.current)>0);
    }

    if(!find_asset_for_platform(json, platform, &info)){
        /* also try common aliases */
        char alt[64];
        if(strcmp(platform,"linux-x86_64")==0) snprintf(alt,sizeof(alt),"linux-x64");
        else if(strcmp(platform,"windows-x86_64")==0) snprintf(alt,sizeof(alt),"windows-x64");
        else if(strcmp(platform,"macos-x86_64")==0) snprintf(alt,sizeof(alt),"macos-x64");
        else alt[0]=0;
        if(!alt[0] || !find_asset_for_platform(json,alt,&info)){
            if(o.check_only || !info.has_update){ /* report no asset but check info still useful */
            } else { free(json); return ELX_UPDATE_ERR_NO_ASSET; }
        }
    }
    if(!find_checksum_url(json, info.asset_name, info.checksum_url, sizeof(info.checksum_url))){
        /* require checksum for automatic install */
        if(!o.check_only && info.has_update && (o.install || !o.check_only)){ free(json); return ELX_UPDATE_ERR_NO_CHECKSUM; }
    }
    free(json);

    if(o.check_only){ if(out_info) *out_info=info; return info.has_update?ELX_UPDATE_AVAILABLE:ELX_UPDATE_UP_TO_DATE; }
    if(!info.has_update){ if(out_info) *out_info=info; return ELX_UPDATE_UP_TO_DATE; }

    /* install path */
    char self[512]; get_self_path(self,sizeof(self));
    if(is_build_tree_binary(self)) return ELX_UPDATE_ERR_NOT_INSTALLED;
    if(access(self, W_OK)!=0) return ELX_UPDATE_ERR_PERMISSIONS;

    if(info.asset_url[0]==0 || !https_url(info.asset_url)) return ELX_UPDATE_ERR_NO_ASSET;
    if(info.checksum_url[0]==0 || !https_url(info.checksum_url)) return ELX_UPDATE_ERR_NO_CHECKSUM;

    char tmpdir[256]; snprintf(tmpdir,sizeof(tmpdir),"/tmp/elx-update-XXXXXX");
    if (!mkdtemp(tmpdir)) return ELX_UPDATE_ERR_INSTALL;
    char asset_tmp[512], sum_tmp[512];
    snprintf(asset_tmp,sizeof(asset_tmp),"%s/asset.bin",tmpdir);
    snprintf(sum_tmp,sizeof(sum_tmp),"%s/asset.sha256",tmpdir);

    const char *apath = g_test_asset_path ? g_test_asset_path : asset_tmp;
    const char *spath = g_test_checksum_path ? g_test_checksum_path : sum_tmp;
    if(!g_test_asset_path){
        st = download_https(info.asset_url, asset_tmp);
        if(st!=ELX_UPDATE_OK){ unlink(asset_tmp); rmdir(tmpdir); return st; }
    }
    if(!g_test_checksum_path){
        st = download_https(info.checksum_url, sum_tmp);
        if(st!=ELX_UPDATE_OK){ unlink(asset_tmp); unlink(sum_tmp); rmdir(tmpdir); return st; }
    }
    size_t slen=0; char *stxt=read_file_all(spath,&slen);
    if(!stxt || !parse_expected_sha256(stxt, info.asset_name, info.expected_sha256)){
        free(stxt); if(!g_test_asset_path) unlink(asset_tmp); if(!g_test_checksum_path) unlink(sum_tmp); rmdir(tmpdir);
        return ELX_UPDATE_ERR_NO_CHECKSUM;
    }
    free(stxt);
    char got[65];
    if(!sha256_file_hex(apath, got) || strcmp(got, info.expected_sha256)!=0){
        if(!g_test_asset_path) unlink(asset_tmp); if(!g_test_checksum_path) unlink(sum_tmp); rmdir(tmpdir);
        return ELX_UPDATE_ERR_CHECKSUM;
    }

    /* SAFE INSTALL: for non-archive executables, atomic replace with backup. For archives (zip/tar),
     * we verify and stage but DO NOT auto-extract/execute; report manual install required.
     * This avoids executing archive contents / complex unsafe extraction. */
    bool is_archive = (strstr(info.asset_name,".zip")||strstr(info.asset_name,".tar.gz")||strstr(info.asset_url,".zip"));
    if(is_archive){
        if(out_info) *out_info=info;
        if(!g_test_asset_path) unlink(asset_tmp); if(!g_test_checksum_path) unlink(sum_tmp); rmdir(tmpdir);
        /* verified download only; user installs package manually */
        return ELX_UPDATE_ERR_INSTALL; /* caller should show "verified; manual install required for archive" */
    }

    char backup[600]; snprintf(backup,sizeof(backup),"%s.bak", self);
    unlink(backup);
    if(rename(self, backup)!=0){
        if(!g_test_asset_path) unlink(asset_tmp); if(!g_test_checksum_path) unlink(sum_tmp); rmdir(tmpdir);
        return ELX_UPDATE_ERR_PERMISSIONS;
    }
    /* copy verified file into place */
    FILE *in=fopen(apath,"rb"), *out=fopen(self,"wb");
    if(!in||!out){
        if(in) fclose(in); if(out) fclose(out); rename(backup,self);
        if(!g_test_asset_path) unlink(asset_tmp); if(!g_test_checksum_path) unlink(sum_tmp); rmdir(tmpdir);
        return ELX_UPDATE_ERR_INSTALL;
    }
    char buf[16384]; size_t n; bool ok=true;
    while((n=fread(buf,1,sizeof(buf),in))>0){ if(fwrite(buf,1,n,out)!=n){ ok=false; break; } }
    fclose(in); fclose(out);
    if(!ok){ rename(backup,self); if(!g_test_asset_path) unlink(asset_tmp); if(!g_test_checksum_path) unlink(sum_tmp); rmdir(tmpdir); return ELX_UPDATE_ERR_INSTALL; }
    chmod(self,0755);

    if(out_info) *out_info=info;
    if(!g_test_asset_path) unlink(asset_tmp); if(!g_test_checksum_path) unlink(sum_tmp); rmdir(tmpdir);
    return ELX_UPDATE_OK;
}
