#!/usr/bin/perl

@psout = `ps ax`;
for ( @psout )
{
	if ( /fxaudio/ )
	{
		$pid = substr($_, 0, 6);
	}
}

kill 'USR1', $pid if ( $pid );
