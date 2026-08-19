### build command

```docker build --target RunnableStage -t asd:runnable . ```

```docker build --target BuildServerCodeStage -t asd:build . ```

Dockerfile contains 3 stages
If you need just runnable use runnable image. It is does not contain dev packages.

If you want to work with codebase execute build image.

```docker run -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --network=host -v ${PWD}:/src --name asd_app asd:runnable bash```


If u see sth like :
```Authorization required, but no authorization protocol specified
qt.qpa.xcb: could not connect to display :1
qt.qpa.plugin: From 6.5.0, xcb-cursor0 or libxcb-cursor0 is needed to load the Qt xcb platform plugin.
qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in "" even though it was found.
This application failed to start because no Qt platform plugin could be initialized. Reinstalling the application may fix this problem.
```

To execute application from container on host linux:
``` xhost +local:* ```

To build project execute following steps

``` mkdir build && cd build ```

``` 
cmake .. -G Ninja -DCMAKE_PREFIX_PATH="/usr/local/Qt/6.5.3/gcc_64/" 
&& cmake --build . --target all 
&& cmake --build . --target VendingMachine 
&& cmake --build . --target package  
```

Empty Gui App Widget app will appear in container /src/build/AppDir/usr/bin.

If that appears means that container contains qt framework and host is able to execute packaged build.


