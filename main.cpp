#include <wasmedge/wasmedge.h>
#include <stdio.h>
#include <iostream>
#include <vector>

const int exit_error_code = 1;
const int exit_success_code = 0;

const char *wasm_file_path = "../module/target/wasm32-wasi/release/funcs.wasm";

int main() {

  WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
  WasmEdge_ConfigureAddHostRegistration(ConfCxt, WasmEdge_HostRegistration_Wasi);

  WasmEdge_StoreContext *StoreCxt = WasmEdge_StoreCreate();
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, StoreCxt);

  // add args/envs/preopens
  // this line start display logs from host functions 
  WasmEdge_ModuleInstanceContext *WasiCxt = WasmEdge_VMGetImportModuleContext(VMCxt, WasmEdge_HostRegistration_Wasi);
  WasmEdge_ModuleInstanceInitWASI(WasiCxt,  NULL, 0, NULL, 0,  NULL, 0);

  WasmEdge_Value P[2], R[1]; 
  WasmEdge_String FuncName;
  WasmEdge_Result Res;

  Res = WasmEdge_VMLoadWasmFromFile(VMCxt, wasm_file_path);
  if (!WasmEdge_ResultOK(Res)) {
    printf("WASM file loading failed\n");
    return exit_error_code;
  }
  Res = WasmEdge_VMValidate(VMCxt);
  if (!WasmEdge_ResultOK(Res)) {
    printf("WASM validation failed\n");
    return exit_error_code;
  }
  Res = WasmEdge_VMInstantiate(VMCxt);
  if (!WasmEdge_ResultOK(Res)) {
    printf("WASM instantiation failed\n");
    return exit_error_code;
  }



  std::vector<std::string> inputs; 
  inputs.push_back("bob");
  //
  int inputs_count = inputs.size();
  //

  // alloc a space for input args
  int pointer_of_pointers = allocate(VMCxt, inputs_count * 4 * 2);
  if (pointer_of_pointers == exit_error_code) {
    printf("[allocate] Error\n");
    return exit_error_code;
  }

  // get active module
  const WasmEdge_ModuleInstanceContext *ActiveModuleCxt = WasmEdge_VMGetActiveModule(VMCxt);
  if (ActiveModuleCxt == NULL) {
    printf("WASM no active module found\n");
    return exit_error_code;
  }
  //
  // get active module's memory
  WasmEdge_String MemoryName = WasmEdge_StringCreateByCString("memory");
  WasmEdge_MemoryInstanceContext *MemoryCxt = WasmEdge_ModuleInstanceFindMemory(ActiveModuleCxt, MemoryName);
  WasmEdge_StringDelete(MemoryName);
  if (MemoryCxt == NULL) {
    printf("WASM no memory found\n");
    return exit_error_code;
  }


  int pos = 0;
  for (auto &inp : inputs) {
    // settle
  }


  // Run func
  P[0] = WasmEdge_ValueGenI32(pointer_of_pointers); // params_pointer: *mut u32
  P[1] = WasmEdge_ValueGenI32(inputs_count); // params_count: i32
  FuncName = WasmEdge_StringCreateByCString("say");
  Res = WasmEdge_VMExecute(VMCxt, FuncName, P, 2, R, 1);
  WasmEdge_StringDelete(FuncName);
  if (WasmEdge_ResultOK(Res)) {
    printf("[say] Ok: %d\n", WasmEdge_ValueGetI32(R[0]));
  } else {
    printf("[say] Error\n");
    return exit_error_code;
  }

  return exit_success_code;
}

// https://stackoverflow.com/questions/27687769/use-different-parameter-data-types-in-same-function-c
std::vector<int> settle(WasmEdge_VMContext *VMCxt, WasmEdge_MemoryInstanceContext *MemoryCxt, std::string param) {
  
  int length = param.length();

  int pointer = allocate(VMCxt, length);

  const char *cParam = param.c_str(); 
  WasmEdge_MemoryInstanceSetData(MemoryCxt, (unsigned char *)cParam, pointer, length);

  std::vector<int> res; 
  res.push_back(pointer);
  res.push_back(length);

  return res;
}

int allocate(WasmEdge_VMContext *VMCxt, int length) {
  WasmEdge_Value P[1], R[1]; 
  WasmEdge_String FuncName;
  WasmEdge_Result Res;

  // alloc a space for input args
  P[0] = WasmEdge_ValueGenI32(length);
  FuncName = WasmEdge_StringCreateByCString("allocate");
  // Don't need to deallocate because the memory will be loaded and free in the wasm
  Res = WasmEdge_VMExecute(VMCxt, FuncName, P, 1, R, 1);
  WasmEdge_StringDelete(FuncName);

  if (WasmEdge_ResultOK(Res)) {
    return WasmEdge_ValueGetI32(R[0]);
  } else {
    return exit_error_code;
  }
}

