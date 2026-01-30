/* src/gpt2_run.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tensorflow/c/c_api.h>

void check_status(TF_Status* status) {
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "ERROR: %s\n", TF_Message(status));
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./gpt2_run <model.pb>\n");
        return 1;
    }

    const char* model_path = argv[1];
    printf("Loading Graph: %s\n", model_path);

    // 1. Initialize Session
    TF_Status* status = TF_NewStatus();
    TF_Graph* graph = TF_NewGraph();
    TF_SessionOptions* opts = TF_NewSessionOptions();
    TF_Session* session = TF_NewSession(graph, opts, status);
    check_status(status);

    // 2. Load Graph Def
    FILE* f = fopen(model_path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    void* data = malloc(fsize);
    fread(data, fsize, 1, f);
    fclose(f);

    TF_ImportGraphDefOptions* import_opts = TF_NewImportGraphDefOptions();
    TF_Buffer* buffer = TF_NewBuffer();
    buffer->data = data;
    buffer->length = fsize;
    TF_GraphImportGraphDef(graph, buffer, import_opts, status);
    check_status(status);

    printf("✅ Graph Loaded Successfully. Ready for Tensors.\n");

    // TODO: 
    // 1. Create Input Tensor (TF_NewTensor) from 'input_context'
    // 2. TF_SessionRun()
    // 3. Read Output Tensor from 'output_logits'

    TF_DeleteBuffer(buffer);
    TF_DeleteImportGraphDefOptions(import_opts);
    TF_DeleteSessionOptions(opts);
    TF_DeleteSession(session, status);
    TF_DeleteGraph(graph);
    TF_DeleteStatus(status);
    return 0;
}
