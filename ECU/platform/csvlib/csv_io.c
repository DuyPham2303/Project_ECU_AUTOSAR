/* platform/csvlib/csv_io.c */
#include "csv_io.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int read_line(FILE* f, char* k, size_t ksz, char* v, size_t vsz){
    char line[512];
    if(!fgets(line, sizeof(line), f)) return -1;
    char* p = strchr(line, ',');
    if(!p) return 1;
    *p = '\0';
    strncpy(k, line, ksz-1); k[ksz-1]='\0';
    strncpy(v, p+1, vsz-1);  v[vsz-1]='\0';
    size_t n = strlen(v);
    while(n && (v[n-1]=='\n' || v[n-1]=='\r')) v[--n]='\0';
    return 0;
}

static int rewrite_with_kv(const char* key, const char* newval){
    FILE* f = fopen(CSV_FILE_PATH, "r");
    if(!f){
        FILE* nf = fopen(CSV_FILE_PATH, "w");
        if(!nf){ printf("[CSV] OPEN-W FAILED(create): %s\n", CSV_FILE_PATH); return -1; }
        fprintf(nf, "%s,%s\n", key, newval);
        fclose(nf);
        printf("[CSV] CREATE & WRITE %s=%s\n", key, newval);
        return 0;
    }
    char outbuf[8192] = {0};
    size_t outlen = 0;
    char k[128], v[512];
    int found = 0;
    while(!feof(f)){
        int r = read_line(f, k, sizeof(k), v, sizeof(v));
        if(r<0) break; if(r>0) continue;
        if(strcmp(k,key)==0){
            outlen += snprintf(outbuf+outlen, sizeof(outbuf)-outlen, "%s,%s\n", key, newval);
            found=1;
        }else{
            outlen += snprintf(outbuf+outlen, sizeof(outbuf)-outlen, "%s,%s\n", k, v);
        }
    }
    fclose(f);
    FILE* nf = fopen(CSV_FILE_PATH, "w");
    if(!nf){ printf("[CSV] OPEN-W FAILED(rewrite): %s\n", CSV_FILE_PATH); return -1; }
    if(!found){
        outlen += snprintf(outbuf+outlen, sizeof(outbuf)-outlen, "%s,%s\n", key, newval);
    }
    fwrite(outbuf, 1, outlen, nf);
    fclose(nf);
    printf("[CSV] WRITE %s=%s (file=%s)\n", key, newval, CSV_FILE_PATH);
    return 0;
}

int csv_getInt(const char* key, int* out){
    if(!key||!out) return -1;
    FILE* f = fopen(CSV_FILE_PATH, "r");
    if(!f){ printf("[CSV] OPEN-R FAILED: %s\n", CSV_FILE_PATH); return -1; }
    char k[128], v[512];
    while(!feof(f)){
        int r = read_line(f, k, sizeof(k), v, sizeof(v));
        if(r<0) break; if(r>0) continue;
        if(strcmp(k,key)==0){
            *out = atoi(v);
            fclose(f);
            printf("[CSV] READ %s=%d\n", key, *out);
            return 0;
        }
    }
    fclose(f);
    printf("[CSV] KEY NOT FOUND: %s\n", key);
    return -2;
}

int csv_setInt(const char* key, int value){
    char buf[64]; snprintf(buf,sizeof(buf), "%d", value);
    return rewrite_with_kv(key, buf);
}

int csv_getString(const char* key, char* buf, size_t buflen){
    if(!key||!buf||!buflen) return -1;
    FILE* f = fopen(CSV_FILE_PATH, "r");
    if(!f){ printf("[CSV] OPEN-R FAILED: %s\n", CSV_FILE_PATH); return -1; }
    char k[128], v[512];
    while(!feof(f)){
        int r = read_line(f, k, sizeof(k), v, sizeof(v));
        if(r<0) break; if(r>0) continue;
        if(strcmp(k,key)==0){
            strncpy(buf, v, buflen-1); buf[buflen-1]='\0';
            fclose(f);
            printf("[CSV] READ %s=\"%s\"\n", key, buf);
            return 0;
        }
    }
    fclose(f);
    printf("[CSV] KEY NOT FOUND: %s\n", key);
    return -2;
}

int csv_setString(const char* key, const char* val){
    if(!key||!val) return -1;
    return rewrite_with_kv(key, val);
}
