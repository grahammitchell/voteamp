/* ogg123.c by Kenneth Arnold <ogg123@arnoldnet.net> */
/* Maintained by Stan Seibert <volsung@xiph.org> */

/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS SOURCE IS GOVERNED BY *
 * THE GNU PUBLIC LICENSE 2, WHICH IS INCLUDED WITH THIS SOURCE.    *
 * PLEASE READ THESE TERMS BEFORE DISTRIBUTING.                     *
 *                                                                  *
 * THE Ogg123 SOURCE CODE IS (C) COPYRIGHT 2000-2001                *
 * by Kenneth C. Arnold <ogg@arnoldnet.net> AND OTHER CONTRIBUTORS  *
 * http://www.xiph.org/                                             *
 *                                                                  *
 ********************************************************************

 last mod: $Id: ogg123.c,v 1.56 2001/12/24 15:58:03 volsung Exp $

 ********************************************************************/

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "audio.h"
#include "buffer.h"
#include "callbacks.h"
#include "cfgfile_options.h"
#include "cmdline_options.h"
#include "format.h"
#include "transport.h"
#include "status.h"
#include "compat.h"

#include "ogg123.h"

void exit_cleanup ();
void play (char *source_string);

/* take buffer out of the data segment, not the stack */
#define AUDIO_CHUNK_SIZE 4096
unsigned char convbuffer[AUDIO_CHUNK_SIZE];
int convsize = AUDIO_CHUNK_SIZE;

ogg123_options_t options;
stat_format_t *stat_format;
buf_t *audio_buffer;

audio_play_arg_t audio_play_arg;

/* Graham Mitchell added this code. */
  char *playlist[255];  /* FIXME: shouldn't have this hardcoded limit
/* End of code added by Graham Mitchell. */


/* ------------------------- config file options -------------------------- */

/* This macro is used to create some dummy variables to hold default values
   for the options. */
#define INIT(type, value) type type##_##value = value
INIT(int, 1);
INIT(int, 0);

file_option_t file_opts[] = {
  /* found, name, description, type, ptr, default */
  {0, "default_device", "default output device", opt_type_string,
   &options.default_device, NULL},
  {0, "shuffle",        "shuffle playlist",      opt_type_bool,
   &options.shuffle,        &int_0},
/* Graham Mitchell added this code. */
  {0, "daemonize",      "run player in background", opt_type_bool,
   &options.daemonize,      &int_0},
/* End of code added by Graham Mitchell. */
  {0, NULL,             NULL,                    0,               NULL,                NULL}
};


/* Flags set by the signal handler to control the threads */
signal_request_t sig_request = {0, 0, 0, 0};


/* ------------------------------- signal handler ------------------------- */


void signal_handler (int signo)
{
  struct timeval tv;
  ogg_int64_t now;
  switch (signo) {
  case SIGINT:

    gettimeofday(&tv, 0);

    /* Units of milliseconds (need the cast to force 64 arithmetics) */
    now = (ogg_int64_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;

    if ( (now - sig_request.last_ctrl_c) <= options.delay)
      sig_request.exit = 1;
    else
      sig_request.skipfile = 1;

    sig_request.last_ctrl_c = now;
    break;

  case SIGTSTP:
    sig_request.pause = 1;
    /* buffer_Pause (Options.outputOpts.buffer);
       buffer_WaitForPaused (Options.outputOpts.buffer);
       }
       if (Options.outputOpts.devicesOpen == 0) {
       close_audio_devices (Options.outputOpts.devices);
       Options.outputOpts.devicesOpen = 0;
       }
    */
    /* open_audio_devices();
       if (Options.outputOpts.buffer) {
       buffer_Unpause (Options.outputOpts.buffer);
       }
    */
    break;

  case SIGCONT:
    break;  /* Don't need to do anything special to resume */
  }
}

/* -------------------------- util functions ---------------------------- */

void options_init (ogg123_options_t *opts)
{
  opts->verbosity = 2;
  opts->shuffle = 0;
/* Graham Mitchell added this code. */
  opts->daemonize = 0;
  opts->status_file = NULL;
/* End of code added by Graham Mitchell. */
  opts->delay = 500;
  opts->nth = 1;
  opts->ntimes = 1;
  opts->seekpos = 0.0;
  opts->buffer_size = 128 * 1024;
  opts->prebuffer = 0.0f;
  opts->input_buffer_size = 64 * 1024;
  opts->input_prebuffer = 50.0f;
  opts->default_device = NULL;

  opts->status_freq = 10.0;
}

/* Graham Mitchell added these functions. */

/* Runs us in the background.  */
void daemonize()
{
  int pid;
  pid = fork();

  if ( pid > 0 )
    exit(0);
  else if ( pid < 0 ) {
    fprintf(stderr, ": unable to fork()\n");
    exit(1);
  }
}

/* Write name of currently playing track to status file.  */
void output_now_playing( char * trackname )
{
  if ( options.status_file ) {
    FILE* fp;
    fp = fopen(options.status_file,"w");
    if ( fp && trackname )
      fprintf(fp, "%s\n", trackname);
    if ( fp )
      fclose(fp);
  }
}

int is_ogg( char * name )
{
  /* Currently pretty naive.  If the name doesn't end in .ogg, it's not one.
     If I bothered to look up more about the format this would be a more
     true check, but this will work for my needs.  Besides, doing it the
     Right Way would require a bit more processing. */

  /* Shooting for "return ( $name =~ m/.ogg$/i );"  */

  /* FIXME: this should at least check for URLs too */

  char * last4 = name + ( strlen(name) - 4 );
  return ( strcasecmp(last4, ".ogg") == 0 );

}


int make_playlist( int argc, char** argv )
{
  int tracks = 0;
  FILE* fp;
  char buf[1024];
  int i;
 
  for ( i=0 ; i < argc ; i++ ) {
    if ( is_ogg(argv[i]) ) {
	  /* Add it to the playlist */
	  playlist[tracks] = argv[i];
	  tracks++;
	} else {
	  /* Assume it's a list; open it and add each line to the big list. */
      fp = fopen(argv[i],"r");
	  if ( fp ) {
	    while ( fgets(buf, 1024, fp) ) {
		  buf[strlen(buf)-1] = '\0';  /* overwrite the trailing '\n' */
	      playlist[tracks] = strdup(buf);
	      tracks++;
		}
        fclose(fp);
      }
	}
  }

  return tracks;
}

/* End of code added by Graham Mitchell. */

/* This function selects which statistics to display for our
   particular configuration.  This does not have anything to do with
   verbosity, but rather with which stats make sense to display. */
void select_stats (stat_format_t *stats, ogg123_options_t *opts, 
		   data_source_t *source, decoder_t *decoder, 
		   buf_t *audio_buffer)
{
  data_source_stats_t *data_source_stats;

  if (audio_buffer != NULL) {
    /* Turn on output buffer stats */
    stats[8].enabled = 1; /* Fill */
    stats[9].enabled = 1; /* State */
  } else {
    stats[8].enabled = 0;
    stats[9].enabled = 0;
  }

  data_source_stats = source->transport->statistics(source);
  if (data_source_stats->input_buffer_used) {
    /* Turn on input buffer stats */
    stats[6].enabled = 1; /* Fill */
    stats[7].enabled = 1; /* State */
  } else {
    stats[6].enabled = 0;
    stats[7].enabled = 0;
  }
    
  /* Put logic here to decide if this stream needs a total time display */
}


/* Handles printing statistics depending upon whether or not we have 
   buffering going on */
void display_statistics (stat_format_t *stat_format,
			 buf_t *audio_buffer, 
			 data_source_t *source,
			 decoder_t *decoder)
{
  print_statistics_arg_t *pstats_arg;
  buffer_stats_t *buffer_stats;

  pstats_arg = new_print_statistics_arg(stat_format,
					source->transport->statistics(source),
					decoder->format->statistics(decoder));

  /* Disable/Enable statistics as needed */

  if (pstats_arg->decoder_statistics->total_time <
      pstats_arg->decoder_statistics->current_time) {
    stat_format[2].enabled = 0;  /* Remaining playback time */
    stat_format[3].enabled = 0;  /* Total playback time */
  }

  if (pstats_arg->data_source_statistics->input_buffer_used) {
    stat_format[6].enabled = 1;  /* Input buffer fill % */
    stat_format[7].enabled = 1;  /* Input buffer state  */
  }

  if (audio_buffer) {
    /* Place a status update into the buffer */
    buffer_append_action_at_end(audio_buffer,
				&print_statistics_action,
				pstats_arg);
    
    /* And if we are not playing right now, do an immediate
       update just the output buffer */
    buffer_stats = buffer_statistics(audio_buffer);
    if (buffer_stats->paused || buffer_stats->prebuffering) {
      pstats_arg = new_print_statistics_arg(stat_format,
					    NULL,
					    NULL);
      print_statistics_action(audio_buffer, pstats_arg);
    }
    free(buffer_stats);
    
  } else
    print_statistics_action(NULL, pstats_arg);
}


void display_statistics_quick (stat_format_t *stat_format,
			       buf_t *audio_buffer, 
			       data_source_t *source,
			       decoder_t *decoder)
{
  print_statistics_arg_t *pstats_arg;

  pstats_arg = new_print_statistics_arg(stat_format,
					source->transport->statistics(source),
					decoder->format->statistics(decoder));

  if (audio_buffer) {
    print_statistics_action(audio_buffer, pstats_arg);
  } else
    print_statistics_action(NULL, pstats_arg);
}


void print_audio_devices_info(audio_device_t *d)
{
  ao_info *info;

  while (d != NULL) {
    info = ao_driver_info(d->driver_id);
    
    status_message(2, "\nDevice:   %s", info->name);
    status_message(2, "Author:   %s", info->author);
    status_message(2, "Comments: %s\n", info->comment);

    d = d->next_device;
  }

}


/* --------------------------- main code -------------------------------- */



int main(int argc, char **argv)
{
  int optind;
/* Graham Mitchell added this variable. */
  int current_track;
  int tracks;
/* End of code added by Graham Mitchell. */

  ao_initialize();
  stat_format = stat_format_create();
  options_init(&options);
  file_options_init(file_opts);

  parse_std_configs(file_opts);
  optind = parse_cmdline_options(argc, argv, &options, file_opts);

  audio_play_arg.devices = options.devices;
  audio_play_arg.stat_format = stat_format;

  /* Don't use status_message until after this point! */
  status_set_verbosity(options.verbosity);

  print_audio_devices_info(options.devices);

  /* Setup signal handlers and callbacks */

  ATEXIT (exit_cleanup);
  signal (SIGINT, signal_handler);
  signal (SIGTSTP, signal_handler);
  signal (SIGCONT, signal_handler);
  signal (SIGALRM, signal_handler);

  /* Do we have anything left to play? */
  if (optind == argc) {
    cmdline_usage();
    exit(1);
  }

/* Graham Mitchell added this code. */

  /* Daemonize if requested */
  if (options.daemonize)
    daemonize();

/* End of code added by Graham Mitchell. */
  
  /* Setup buffer */ 
  if (options.buffer_size > 0) {
    audio_buffer = buffer_create(options.buffer_size,
				 options.buffer_size * options.prebuffer / 100,
				 audio_play_callback, &audio_play_arg,
				 AUDIO_CHUNK_SIZE);
    if (audio_buffer == NULL) {
      status_error("Error: Could not create audio buffer.\n");
      exit(1);
    }
  } else
    audio_buffer = NULL;

/* Graham Mitchell added this code. */

  /* Generate global playlist */
  tracks = make_playlist( argc-optind+1, &argv[optind-1] );

/* End of code added by Graham Mitchell. */

  /* Shuffle playlist */
  if (options.shuffle) {
    int i;
    
    srandom(time(NULL));
    
/* Graham Mitchell modified this code. */

    for (i = 0; i < tracks; i++) {
      int j = i + random() % (tracks - i);
      char *temp = playlist[i];
      playlist[i] = playlist[j];
      playlist[j] = temp;
    }
  }

  /* Play the files/streams */

  current_track = 0;
  while (current_track < tracks && !sig_request.exit) {
    play(playlist[current_track]);
    current_track++;
  }

/* End of code modified by Graham Mitchell. */

/* Graham Mitchell added this code. */
  if ( options.status_file )
    output_now_playing( NULL );
/* End of code added by Graham Mitchell. */

  exit (0);
}


void play (char *source_string)
{
  transport_t *transport;
  format_t *format;
  data_source_t *source;
  decoder_t *decoder;

  decoder_callbacks_t decoder_callbacks;
  void *decoder_callbacks_arg;

  /* Preserve between calls so we only open the audio device when we 
     have to */
  static audio_format_t old_audio_fmt = { 0, 0, 0, 0, 0 };
  audio_format_t new_audio_fmt;
  audio_reopen_arg_t *reopen_arg;

  /* Flags and counters galore */
  int eof = 0, eos = 0, ret;
  int nthc = 0, ntimesc = 0;
  int next_status = 0;
  int status_interval = 0;


  /* Set preferred audio format (used by decoder) */
  new_audio_fmt.big_endian = ao_is_big_endian();
  new_audio_fmt.signed_sample = 1;
  new_audio_fmt.word_size = 2;

  /* Select appropriate callbacks */
  if (audio_buffer != NULL) {
    decoder_callbacks.printf_error = &decoder_buffered_error_callback;
    decoder_callbacks.printf_metadata = &decoder_buffered_error_callback;
    decoder_callbacks_arg = audio_buffer;
  } else {
    decoder_callbacks.printf_error = &decoder_error_callback;
    decoder_callbacks.printf_metadata = &decoder_error_callback;
    decoder_callbacks_arg = NULL;
  }

  /* Locate and use transport for this data source */  
  if ( (transport = select_transport(source_string)) == NULL ) {
    status_error("No module could be found to read from %s.\n", source_string);
    return;
  }
  
  if ( (source = transport->open(source_string, &options)) == NULL ) {
    status_error("Cannot open %s.\n", source_string);
    return;
  }

  /* Detect the file format and initialize a decoder */
  if ( (format = select_format(source)) == NULL ) {
    status_error("The file format of %s is not supported.\n", source_string);
    return;
  }
  
  if ( (decoder = format->init(source, &options, &new_audio_fmt, 
			       &decoder_callbacks,
			       decoder_callbacks_arg)) == NULL ) {
    status_error("Error opening %s using the %s module."
		 "  The file may be corrupted.\n", source_string,
		 format->name);
    return;
  }

  /* Decide which statistics are valid */
  select_stats(stat_format, &options, source, decoder, audio_buffer);


  /* Reset all of the signal flags */
  sig_request.skipfile = 0;
  sig_request.exit     = 0;
  sig_request.pause    = 0;

  /* Start the audio playback thread before we begin sending data */    
  if (audio_buffer != NULL) {
    
    /* First reset mutexes and other synchronization variables */
    buffer_reset (audio_buffer);
    buffer_thread_start (audio_buffer);
  }

  /* Show which file we are playing */
  decoder_callbacks.printf_metadata(decoder_callbacks_arg, 1,
				    "Playing: %s", source_string);

/* Graham Mitchell added this code. */
  if ( options.status_file )
    output_now_playing( source_string );
/* End of code added by Graham Mitchell. */

  /* Skip over audio */
  if (options.seekpos > 0.0) {
    if (!format->seek(decoder, options.seekpos, DECODER_SEEK_START))
      status_error("Could not skip %f seconds of audio.", options.seekpos);
  }

  /* Main loop:  Iterates over all of the logical bitstreams in the file */
  while (!eof && !sig_request.exit) {
    
    /* Loop through data within a logical bitstream */
    eos = 0;    
    while (!eos && !sig_request.exit) {
      
      /* Check signals */
      if (sig_request.skipfile) {
	eof = eos = 1;
	break;
      }

      if (sig_request.pause) {
	if (audio_buffer)
	  buffer_thread_pause (audio_buffer);

	kill (getpid(), SIGSTOP); /* We block here until we unpause */
	
	/* Done pausing */
	if (audio_buffer)
	  buffer_thread_unpause (audio_buffer);

	sig_request.pause = 0;
      }


      /* Read another block of audio data */
      ret = format->read(decoder, convbuffer, convsize, &eos, &new_audio_fmt);

      /* Bail if we need to */
      if (ret == 0) {
	eof = eos = 1;
	break;
      } else if (ret < 0) {
	status_error("Error: Decoding failure.\n");
	break;
      }

      
      /* Check to see if the audio format has changed */
      if (!audio_format_equal(&new_audio_fmt, &old_audio_fmt)) {
	old_audio_fmt = new_audio_fmt;
	
	/* Update our status printing interval */
	status_interval = new_audio_fmt.word_size * new_audio_fmt.channels * 
	  new_audio_fmt.rate / options.status_freq;
	next_status = 0;

	reopen_arg = new_audio_reopen_arg(options.devices, &new_audio_fmt);

	if (audio_buffer)	  
	  buffer_insert_action_at_end(audio_buffer, &audio_reopen_action,
				      reopen_arg);
	else
	  audio_reopen_action(NULL, reopen_arg);
      }
      

      /* Update statistics display if needed */
      if (next_status <= 0) {
	display_statistics(stat_format, audio_buffer, source, decoder); 
	next_status = status_interval;
      } else
	next_status -= ret;


      /* Write audio data block to output, skipping or repeating chunks
	 as needed */
      do {
	
	if (nthc-- == 0) {
	  if (audio_buffer)
	    buffer_submit_data(audio_buffer, convbuffer, ret);
	  else
	    audio_play_callback(convbuffer, ret, eos, &audio_play_arg);
	  
	  nthc = options.nth - 1;
	}
	
      } while (++ntimesc < options.ntimes);

      ntimesc = 0;
            
    } /* End of data loop */
    
  } /* End of logical bitstream loop */
  
  /* Done playing this logical bitstream.  Clean up house. */

  if (audio_buffer) {
    
    if (!sig_request.exit && !sig_request.skipfile) {
      buffer_mark_eos(audio_buffer);
      buffer_wait_for_empty(audio_buffer);
    }

    buffer_thread_kill(audio_buffer);
  }

  /* Print final stats */
  display_statistics_quick(stat_format, audio_buffer, source, decoder); 
   
  
  format->cleanup(decoder);
  transport->close(source);
  status_reset_output_lock();  /* In case we were killed mid-output */

  status_message(1, "Done.");
  
  if (sig_request.exit)
    exit (0);
}


void exit_cleanup ()
{
      
  if (audio_buffer != NULL) {
    buffer_destroy (audio_buffer);
    audio_buffer = NULL;
  }

  ao_onexit (options.devices);
}
