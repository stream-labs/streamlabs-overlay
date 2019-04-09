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
              "UNICODE",
              "_UNICODE "
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "WarningLevel": 3,
                "ExceptionHandling": 1
              }
            }
          }
        ]
      ]
    }
  ]
}