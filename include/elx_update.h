/*
 * ELX Update System — ELECTRON VISION (ELX SDK)
 * Safe updater for official GitHub Releases. No auto root, no curl|bash,
 * HTTPS only, SHA-256 required before install, atomic swap + rollback.
 */
#ifndef ELX_UPDATE_H
#define ELX_UPDATE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ELX_UPDATE_OK = 0,
    ELX_UPDATE_UP_TO_DATE,
    ELX_UPDATE_AVAILABLE,
    ELX_UPDATE_ERR_NETWORK,
    ELX_UPDATE_ERR_API,
    ELX_UPDATE_ERR_RATE_LIMIT,
    ELX_UPDATE_ERR_NO_RELEASE,
    ELX_UPDATE_ERR_UNSUPPORTED_PLATFORM,
    ELX_UPDATE_ERR_NO_ASSET,
    ELX_UPDATE_ERR_NO_CHECKSUM,
    ELX_UPDATE_ERR_DOWNLOAD,
    ELX_UPDATE_ERR_CHECKSUM,
    ELX_UPDATE_ERR_INSTALL,
    ELX_UPDATE_ERR_PERMISSIONS,
    ELX_UPDATE_ERR_NOT_INSTALLED,
    ELX_UPDATE_ERR_VERSION,
    ELX_UPDATE_ERR_UNSAFE
} elx_update_status_t;

typedef struct {
    char current[64];
    char latest[64];
    char tag[64];
    char published_at[64];
    char release_url[512];
    char notes[1024];
    char asset_name[256];
    char asset_url[512];
    char checksum_url[512];
    char expected_sha256[65];
    bool has_update;
    bool is_prerelease;
} elx_update_info_t;

typedef struct {
    bool check_only;
    bool install;
    const char *target_version; /* e.g. 0.4.2 or 0.4.2-phase4; NULL = latest stable */
    bool json; /* future: machine output; currently informational */
} elx_update_options_t;

const char *elx_update_status_string(elx_update_status_t st);

/* Parse owner/repo from git remote origin URL. Returns false if not GitHub or missing. */
bool elx_update_parse_github_remote(const char *origin_url, char *owner, size_t owner_sz,
                                    char *repo, size_t repo_sz);

/* Detect official repo (env ELX_UPDATE_REPO=owner/repo or git remote origin). */
bool elx_update_detect_repo(char *owner, size_t owner_sz, char *repo, size_t repo_sz);

/* Compare ELX versions (supports X.Y.Z and -phaseN). Returns <0 if a<b, 0 eq, >0 if a>b. */
int elx_update_compare_versions(const char *a, const char *b);

/* Platform/arch id for asset matching (e.g. linux-x86_64, windows-x64, macos-arm64). */
void elx_update_platform_id(char *out, size_t out_sz);

/* High-level: check/install. out_info may be NULL. */
elx_update_status_t elx_update_run(const elx_update_options_t *opts, elx_update_info_t *out_info);

/* Parse a SHA-256 checksum manifest for an exact asset filename.
 * Supported line formats (per line): "<64hex>  <filename>", "<64hex> *<filename>",
 * and "<64hex><whitespace><filename>". Matching requires the filename field to equal
 * the requested asset_name exactly (no arbitrary fallback). Returns false on missing,
 * malformed, duplicate, or ambiguous match, or when no filename can be bound.
 * out_hex (65 bytes) receives lowercase hex + NUL on success. */
bool elx_update_parse_sha256_manifest(const char *checksum_text, const char *asset_name,
                                      char out_hex[65]);

/* Test hooks (unit tests): inject API JSON and asset/checksum file paths; no network. */
void elx_update_set_test_api_json(const char *json, size_t len);
void elx_update_set_test_asset_path(const char *path);
void elx_update_set_test_checksum_path(const char *path);
void elx_update_clear_test_hooks(void);

#ifdef __cplusplus
}
#endif
#endif
