# JavaScriptCore for Java

JNI implementation for JavaScriptCore API

## Requirements

- [gradle](http://www.gradle.org/) 
- MacOS X SDK and XCode

## How to build and test for OSX

```
$ gradle clean
$ gradle osxbuild
$ gradle test
```

# How to build for Android

Make sure NDK_MODULE_PATH is properly set to your BUILD_webkit2 JavaScriptCore module directory

```
$ gradle clean
$ gradle androidbuild
$ ls -l native/android/libs/armeabi-v7a/*.so
```


## Unimplemented APIs

- [JSStringRef](https://developer.apple.com/library/mac/documentation/JavaScriptCore/Reference/JSStringRef_header_reference/Reference/reference.html), these functions are all replaced by Java String.

