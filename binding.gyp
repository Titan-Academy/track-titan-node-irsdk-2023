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
      ],
    }
  ]
}