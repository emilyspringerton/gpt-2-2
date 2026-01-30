/* src/download_model.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/types.h>

/* File list from original python script */
const char* FILES[] = {
    "checkpoint",
    "encoder.json",
    "hparams.json",
    "model.ckpt.data-00000-of-00001",
    "model.ckpt.index",
    "model.ckpt.meta",
    "vocab.bpe"
};
const int NUM_FILES = 7;
const char* BASE_URL = "https://openaipublic.blob.core.windows.net/gpt-2";

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./download_model <model_name>\nExample: ./download_model 124M\n");
        return 1;
    }

    const char* model = argv[1];
    char subdir[256];
    snprintf(subdir, sizeof(subdir), "models/%s", model);

    /* Create directory logic (mkdir -p models/124M) */
    #ifdef _WIN32
        _mkdir("models");
        _mkdir(subdir);
    #else 
        mkdir("models", 0777);
        mkdir(subdir, 0777);
    #endif

    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        for (int i = 0; i < NUM_FILES; i++) {
            char url[512];
            char outfilename[512];
            
            /* Construct URL: BASE_URL/models/124M/filename */
            snprintf(url, sizeof(url), "%s/models/%s/%s", BASE_URL, model, FILES[i]);
            snprintf(outfilename, sizeof(outfilename), "%s/%s", subdir, FILES[i]);

            printf("⬇️  Downloading %s...\n", FILES[i]);

            FILE *fp = fopen(outfilename, "wb");
            if (!fp) {
                fprintf(stderr, "❌ Error opening file %s for writing\n", outfilename);
                return 1;
            }

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // Fail on 404

            res = curl_easy_perform(curl);
            
            if (res != CURLE_OK) {
                fprintf(stderr, "❌ curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                fclose(fp);
                return 1;
            }
            
            fclose(fp);
        }
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    printf("✅ All files downloaded successfully to %s/\n", subdir);
    return 0;
}
