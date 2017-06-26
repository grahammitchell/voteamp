#!/usr/bin/perl

$election_duration = 96 * 60;

while ( 1 )
{
	sleep(60);
	$current = time();
	if ( $current >= $next )
	{
		@psout = `ps ax`;
		for ( @psout )
		{
			if ( m#voted# )
			{
				$pid = substr($_, 0, 6);
			}
		}
		$next = $current + $election_duration;

		kill 'HUP', $pid     if ( $pid );
	}
}
