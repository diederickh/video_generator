/*

  video_generator
  -------------

  Creates a test video signal with 7 colors vertical bars
  and one moving horizontal. First initialize the generator using
  `video_generator_init()` then call `video_generator_update()`. Each time you
  call `video_generator_update()` it will update the frame and y, u, v planes.
  When ready clean memory using `video_generator_clear()`.
  
  You can write the frames into a file and use avconv to encode it to some format:

  ````sh
  ./avconv -f rawvideo -pix_fmt yuv420p -s 640x480 -i output.yuv -vcodec h264 -r 25 -y out.mov
  ````

  TODO
  -----
  - There are many things we could optimise.
  
  Example: 
  --------
  <example>

     fp = fopen("output.yuv", "wb");

     video_generator gen;

     if (video_generator_init(&gen, WIDTH, HEIGHT, FPS) < 0) {
       printf("Error: cannot initialize the generator.\n");
       exit(1);
     }

     while(1) {

        printf("Frame: %llu\n", gen.frame);

        video_generator_update(&gen);

        // write planes to a file
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

  </example>
 */

#ifndef VIDEO_GENERATOR_H
#define VIDEO_GENERATOR_H

#define RXS_MAX_CHARS 11
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct video_generator video_generator;
typedef struct video_generator_char video_generator_char;

struct video_generator_char {
  int id;
  int x;
  int y;
  int width;
  int height;
  int xoffset;
  int yoffset;
  int xadvance;
};

struct video_generator {
  uint64_t frame;
  uint8_t* y;
  uint8_t* u;
  uint8_t* v;
  uint32_t width;
  uint32_t height;
  uint32_t ybytes;
  uint32_t ubytes;
  uint32_t vbytes;
  uint32_t nbytes;
  uint8_t* planes[3];                       /* pointers to the planes (similar to the y, u and v members) */ 
  uint32_t strides[3];                      /* strides for the separate planes. */
  int fps_num;
  int fps_den;
  double fps;                               /* in microseconds, 1 fps == 1.000.000 us */
  double step;                              /* used to create the moving bar */
  double perc;                              /* position of the moving bar */
  video_generator_char chars[RXS_MAX_CHARS]; /*  bitmap characters, `0-9` and `:` */
  int font_w;
  int font_h;
  int font_line_height;
};

int video_generator_init(video_generator* g, int width, int height, int fps);
int video_generator_update(video_generator* g);
int video_generator_clear(video_generator* g);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
