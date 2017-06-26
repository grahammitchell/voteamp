#!/usr/bin/perl

@psout = `ps ax`;
for ( @psout )
{
	if ( /voted/ )
	{
		$pid = substr($_, 0, 6);
	}
}

kill 'HUP', $pid if ( $pid );
