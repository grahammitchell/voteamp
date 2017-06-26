/* Graham Mitchell added this code. */
/* End of code added by Graham Mitchell. */

void output_now_playing( char * trackname )
{
	if ( now_playing )
	{
		FILE* fp;
		fp = fopen(now_playing_buf,"w");
		if ( fp && trackname )
			fprintf(fp, "%s\n", trackname);
		if ( fp )
			close(fp);
	}
}


void daemonize()
{
	int pid;
	pid = fork();

	if ( pid > 0 )
		exit(0);
	else if ( pid < 0 )
	{
		fprintf(STDERR, "PROGRAM_NAME: unable to fork()\n");
		exit(1);
	}
}

