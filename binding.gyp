{
  "targets": [
    {
      "target_name": "streamlabs_overlay",
      "include_dirs": [
        "include"
      ],
      "sources": [
        "src\*.cpp"
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
            "libraries": ['-ld2d1.lib '],
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