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

/* ----------------------------------------------------------------------------------- */
/*                          T H R E A D I N G                                          */
/* ----------------------------------------------------------------------------------- */

  /* ------------------------------------------------------------------------- */

  struct thread;                                                 /* Forward declared. */
  struct mutex;                                                  /* Forward declared. */
  typedef struct thread thread;
  typedef struct mutex mutex;
  typedef void*(*thread_function)(void* param);                  /* The thread function you need to write. */

  thread* thread_alloc(thread_function func, void* param);       /* Create a new thread handle. Don't forget to call thread_free(). */ 
  int thread_free(thread* t);                                    /* Frees the thread that was allocated by `thread_alloc()` */   
  int thread_join(thread* t);                                    /* Join the thread. */
  int mutex_init(mutex* m);                                      /* Initialize a mutex. */
  int mutex_destroy(mutex* m);                                   /* Destroy the mutex. */
  int mutex_lock(mutex* m);                                      /* Lock the mutex. */
  int mutex_unlock(mutex* m);                                    /* Unlock the mutex. */

  /* ------------------------------------------------------------------------- */

#if defined(_WIN32)

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    struct thread {
      HANDLE handle;
      DWORD thread_id;
      thread_function func;
      void* user;
    };
    
    struct mutex {
      HANDLE handle;
    };
    
    DWORD WINAPI thread_wrapper_function(LPVOID param);

#elif defined(__linux) || defined(__APPLE__)

    #include <string.h>
    #include <stdlib.h>
    #include <pthread.h>
    
    struct thread {
      pthread_t handle;
      thread_function func;
      void* user; 
    };
    
    struct mutex {
      pthread_mutex_t handle;
    };
    
    void* thread_function_wrapper(void* t);

#endif

/* ----------------------------------------------------------------------------------- */
/*                          V I D E O   G E N E R A T O  R                             */
/* ----------------------------------------------------------------------------------- */

typedef struct video_generator video_generator;
typedef struct video_generator_char video_generator_char;

/* 
   When we generate audio we do this from a separate thread to make sure we
   simulate a real audio input system that e.g. captures audio from a microphone. 
   Also this means that the user doesn't need to call a update() function or something
   that would normally be used to retrieve audio samples. 

   Make sure that you don't do too much in the callback because we need to keep up
   with the samplerate.

   @param samples        The samples that you need to process. 
   @param nbytes         The number of bytes in `samples`
   @param nframes        The number of frames in `samples`.   
*/
typedef void(*video_generator_audio_callback)(const int16_t* samples, uint32_t nbytes, uint32_t nframes); 

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
  uint8_t* planes[3];                                     /* pointers to the planes (similar to the y, u and v members) */ 
  uint32_t strides[3];                                    /* strides for the separate planes. */
  int fps_num;                                            
  int fps_den;                                            
  double fps;                                             /* in microseconds, 1 fps == 1.000.000 us */
  double step;                                            /* used to create the moving bar */
  double perc;                                            /* position of the moving bar */
  video_generator_char chars[RXS_MAX_CHARS];              /* bitmap characters, `0-9` and `:` */
  int font_w;
  int font_h;
  int font_line_height;

  /* Audio: 2 channel, 44100hz, int16, intereaved 2 channels*/
  uint16_t audio_nchannels;                               /* for now always: 2 */
  uint8_t audio_nseconds;                                 /* the number of seconds of audio we have in the audio_buffer. Always 4. */
  uint16_t audio_samplerate;                              /* for now always: 44100 */
  uint16_t audio_bip_frequency;                           /* frequency for the bip sound, 600hz. */
  uint16_t audio_bop_frequency;                           /* frequency for the bop sound, 300hz. */
  uint32_t audio_bip_millis;                              /* number of millis for the bip sound */ 
  uint32_t audio_bop_millis;                              /* number of millis for the bop sound */ 
  uint32_t audio_nbytes;                                  /* number of bytes in audio_buffer. */
  int16_t* audio_buffer;                                  /* this will contain the audio samples */
  video_generator_audio_callback audio_callback;          /* will be called from the thread when the user needs to process audio. */
  thread* audio_thread;                                   /* the audio callback is called from another thread to simulate microphone input.*/
  mutex audio_mutex;                                      /* used to sync. shared data */
  uint8_t audio_thread_must_stop;                         /* is set to 1 when the thread needs to stop */
  uint8_t audio_is_bip;                                   /* is set to 1 as soon as the bip audio part it passed into the callback. */
  uint8_t audio_is_bop;                                   /* is set to 1 as soon as the bop audio part is passed into the callback. */ 
};

int video_generator_init(video_generator* g, 
                         int width, 
                         int height, 
                         int fps, 
                         video_generator_audio_callback audiocb
                        );
int video_generator_update(video_generator* g);
int video_generator_clear(video_generator* g);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
