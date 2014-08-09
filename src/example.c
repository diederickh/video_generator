#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <video_generator.h>

static video_generator gen;

int main() {

  printf("\n\nVideo Generator.\n\n");

  if (video_generator_init(&gen, 320, 240, 25) < 0) {
    printf("Error: cannot initialize the video generator.\n");
    exit(1);
  }

  while(1) {
    video_generator_update(&gen);
    usleep(gen.fps);

    /* 
       Do something with the generated data:
       encode(gen.planes, gen.strides, gen.frame);
     */

    printf("Frame: %llu\n", gen.frame);
  }
  return 0;
}
