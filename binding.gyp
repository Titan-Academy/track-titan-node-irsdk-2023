{
  "targets": [
    {
      "target_name": "IrSdkNodeBindings",
      "cflags": [ "-Wall", "-std=c++11" ],
      "sources": [ 
        "src/cpp/IrSdkNodeBindings.cpp", 
        "src/cpp/IrSdkCommand.cpp", 
        "src/cpp/IRSDKWrapper.cpp", 
        "src/cpp/IrSdkBindingHelpers.cpp" 
      ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ],
      "dependencies": [
         "<!(node -p \"require('node-addon-api').targets\"):node_addon_api",
      ],
      "defines": [
        "NAPI_VERSION=<(napi_build_version)",
      ],
      "default_configuration": "Release",
      "configurations": {
        "Release": { 
          "msvs_settings": {
            "VCCLCompilerTool": {
                "ExceptionHandling": 1
            }
          }
        }
      }
    },
      {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}