{
  "targets": [
    {
      "target_name": "node-dpapi",
      "sources": [ "src/node-dpapi.cpp" ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "include"
      ],
      "libraries": [
        "-lcrypt32.lib"
      ]
    }
  ]
}