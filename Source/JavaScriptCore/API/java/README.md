# JavaScriptCore for Java

JNI implementation for JavaScriptCore API

## Requirements

- [gradle](http://www.gradle.org/) 
- MacOS X SDK and XCode

## How to build and test

```
$ gradle clean
$ gradle jnibuild
$ gradle test
```

## Unimplemented APIs

- [JSStringRef](https://developer.apple.com/library/mac/documentation/JavaScriptCore/Reference/JSStringRef_header_reference/Reference/reference.html), these functions are all replaced by Java String.
- [JSClassCreate and JSClassDefinition](https://developer.apple.com/library/mac/documentation/JavaScriptCore/Reference/JSObjectRef_header_reference/Reference/reference.html), JSClassDefinition needs function pointer for callbacks and you can't simply map them with Java methods.

