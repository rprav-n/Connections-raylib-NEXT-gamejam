#make -e PLATFORM=PLATFORM_WEB -B

emcc -o main.html main.c -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -I. -I C:/raylib/raylib/src -I C:/raylib/raylib/src/external -L. -L C:/raylib/raylib/src -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 -s FULL_ES2=1 -s FULL_ES3=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 --shell-file C:/raylib/raylib/src/shell.html C:/raylib/raylib/src/web/libraylib.a -DPLATFORM_WEB -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' -s EXPORTED_RUNTIME_METHODS=ccall --preload-file ../assets

#python -m http.server