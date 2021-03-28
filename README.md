# webgpu-cpp-hello-triangle

Simple hello triangle example using the current API of WebGPU using C++/WASM and Emscripten.

Following is an example output of the html canvas:

![example output](output.png "example output" )

# build
To build run the following command from the root directory.
```
emcc main.cpp -o index.html -g -g4 -s USE_WEBGPU=1 --shell-file shell.html --bind -std=c++20
```

# run

Once the output is present, run a local web server from the root folder. As example you can use Python3 for this:
```
python -m http.server 8000
```

Then simply visit `http://localhost:8000/index.html` in your browser of choice that supports WebGPU. (Chrome Canary was used for testing this repo)

To see an example output, you can visit the [Github Pages](https://jessydl.github.io/webgpu-cpp-hello-triangle/) page for this repository.
