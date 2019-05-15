{
  "targets": [
    {
      "target_name": "streamlabs_overlay",
      "include_dirs": [
        "src",
        "include",
        "webform"
      ],
      "sources": [
        "src\*.cpp",
        "src\*.h",
        "webform\*.cpp",
        "node_module\*.cpp",
        "node_module\*.h"
      ],
      "conditions": [
        [
          "OS=='win'",
          {
            "defines": [
              '_CRT_SECURE_NO_DEPRECATE',
              '_CRT_NONSTDC_NO_DEPRECATE',
              "UNICODE",
              "_UNICODE "
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "WarningLevel": 3,
                'AdditionalOptions': ['/permissive-', '/std:c++17'],
                "ExceptionHandling": 1
              }
            }
          }
        ]
      ]
    }
  ]
}