### build command

```docker build --target RunnableStage -t asd:runnable . ```

```docker build --target BuildServerCodeStage -t asd:build . ```

Dockerfile contains 3 stages
If you need just runnable use runnable image. It is does not contain dev packages.

If you want to work with codebase execute build image.

```docker run -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --network=host -v ${PWD}:/src --name asd_app asd:runnable bash```
