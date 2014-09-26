#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <video_generator.h>

uint64_t now = 0;
uint64_t timeout = 0;
uint64_t total_audio_frames = 0;
video_generator gen;
uint8_t must_run = 1;

/* ----------------------------------------------------------------------------------- */

static void on_audio(const int16_t* samples, uint32_t nbytes, uint32_t nframes);
static void on_sigh(int s);

/* ----------------------------------------------------------------------------------- */

int main() {
  uint64_t delay ;
  printf("\n\nVideo Generator.\n\n");

  if (video_generator_init(&gen, 320, 240, 25, on_audio) < 0) {
    printf("Error: cannot initialize the video generator.\n");
    exit(1);
  }

  /* write raw audio to a file. */
  FILE* fp = fopen("out_s16_44100_stereo.pcm", "wb");
  if (!fp) {
    printf("Error: cannot open pcm output file.\n");
    exit(1);
  }
  fwrite(gen.audio_buffer, gen.audio_nbytes, 1, fp);
  fclose(fp);

  signal(SIGINT, on_sigh);

  delay = ((uint64_t)gen.fps) * 1e3; /* from us to ns. */
  timeout = 0;

  while(must_run) {

    if (now > timeout) {
      timeout = now + delay;
      video_generator_update(&gen);
      printf("Frame: %llu\n", gen.frame);
    }
  }
    
  video_generator_clear(&gen);

  return 0;
}

/* ----------------------------------------------------------------------------------- */

static void on_sigh(int s) {
  printf("Got signal, stopping.\n");
  must_run = 0;
}

static void on_audio(const int16_t* samples, uint32_t nbytes, uint32_t nframes) {
  total_audio_frames += nframes; /* this is our timebase */
  now = ((1.0 / 44100.0) * 1e9) * total_audio_frames;
}
