#!/usr/bin/perl

@psout = `ps ax`;

for ( @psout )
{
	if ( /listmaker/ or /voted/ or /fxaudio/ )
	{
		$pid = substr($_, 0, 6);
		push @goners, $pid;
	}
}

kill 'TERM', @goners;
