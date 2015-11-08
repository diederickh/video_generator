#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <video_generator.h>

/* ----------------------------------------------------------------------------------- */

static void on_audio(const int16_t* samples, uint32_t nbytes, uint32_t nframes);
static void on_sigh(int s);

/* ----------------------------------------------------------------------------------- */

uint64_t now = 0;
uint64_t total_audio_frames = 0;
video_generator_settings cfg;
video_generator gen;
uint8_t must_run = 1;
uint64_t goal_frame = 0;
uint64_t goal_copy = 0;

/* ----------------------------------------------------------------------------------- */

int main() {

  FILE* video_fp = NULL;

  printf("\n\nVideo Generator.\n\n");

  /* Set the video generator settings. */
  cfg.width = 800;
  cfg.height = 600;
  cfg.fps = 25;
  cfg.audio_callback = on_audio;
  cfg.bip_frequency = 500;
  cfg.bop_frequency = 1500;

  if (0 != video_generator_init(&cfg, &gen)) {
    printf("Error: cannot initialize the video generator.\n");
    exit(EXIT_FAILURE);
  }

  /* Write raw audio block to a file. This chunk will be repeadetly used while running. */
  FILE* fp = fopen("out_s16_44100_stereo.pcm", "wb");
  if (!fp) {
    printf("Error: cannot open pcm output file.\n");
    exit(EXIT_FAILURE);
  }

  if (1 != fwrite(gen.audio_buffer, gen.audio_nbytes, 1, fp)) {
    printf("Error: failed to write the audio block.\n");
    exit(EXIT_FAILURE);
  }

  if (0 != fclose(fp)) {
    printf("Error: failed to close the audio example file.\n");
  }

  /* Write video to a raw yuv file. */
  video_fp = fopen("out_yuv420p_320x240.yuv", "wb");
  if (!video_fp) {
    printf("Error: failed to open the video output file.");
  }

  signal(SIGINT, on_sigh);

  while(must_run) {

    goal_copy = goal_frame;

    while (gen.frame < goal_copy) {

      video_generator_update(&gen);

      if (1 != fwrite((char*)gen.y, gen.nbytes, 1, video_fp)) {
        printf("Failed to write frame %llu to file.\n", gen.frame);
      }

      printf("Frame: %llu\n", gen.frame);
    }
  }
    
  video_generator_clear(&gen);

  if (0 != fclose(video_fp)) {
    printf("Error: failed to close the video file correctly.\n");
  }

  printf("Ready.\n");

  return 0;
}

/* ----------------------------------------------------------------------------------- */

static void on_sigh(int s) {
  printf("\nGot signal, stopping.\n");
  must_run = 0;
}

static void on_audio(const int16_t* samples, uint32_t nbytes, uint32_t nframes) {
  total_audio_frames += nframes; /* this can be used for our timebase */
  now = ((1.0 / 44100.0) * 1e9) * total_audio_frames; /* not used in this example but this could be used as your timebase. */
  goal_frame = now / ((uint64_t)gen.fps * 1e3); /* set the goal frame up to which we have to generate frames. */
}
