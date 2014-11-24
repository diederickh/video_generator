Video Generator
===============

The "Video Generator" was created to test long running video and audio encoders
and video/audio sync. It can generate a continous stream of YUV420P video frames
with a 44100hz, int16 2 channel audio signal. 

See video_generator.h for a description on how to use it or take a look at 
the example.c file which contains a basic example of how to use the video generator
to generate an video and audio signal.

The generated video contains 7 vertical bars and 1 horizontal scrolling one. The 
horizontal bar moves from to to bottom every 5 seconds. In the center of the video
you see a black rectangle which displays the time. This time is based on the number
of generated frames and framerate. It's up to the user to generate enough frames 
to have a stable framerate.

Compiling
----------
You can either just include the `video_generator.c` file in your project or use
the accompanying `CMakeLists.txt` and `release.sh` files. If you want to make use
of cmake, make sure that you've installed it. 

On Mac, Linux and Windows use the following. For windows users, make sure to 
execute the `./release.sh` script using a Git Bash shell.

````sh
cd build
./release.sh
````

The example is installed into `install/[system-triplet]/bin/`.



Example
-------

````c++

     fp = fopen("output.yuv", "wb");

     video_generator gen;
     video_geneator_settings cfg;
     
     cfg.width = WIDTH;
     cfg.height = HEIGHT;
     cfg.fps = FPS;
     
     if (0 != video_generator_init(&cfg, &gen)) {
       printf("Error: cannot initialize the generator.\n");
       exit(1);
     }

     while(1) {

        printf("Frame: %llu\n", gen.frame);

        video_generator_update(&gen);

        // write video planes to a file
        fwrite((char*)gen.y, gen.ybytes,1,  fp);
        fwrite((char*)gen.u, gen.ubytes,1, fp);
        fwrite((char*)gen.v, gen.vbytes,1, fp);

        if (gen.frame > 250) { 
          break;
        }

        usleep(gen.fps);
     }

    fclose(fp);

    video_generator_clear(&gen);

````