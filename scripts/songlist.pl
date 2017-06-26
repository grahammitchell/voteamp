#!/usr/bin/perl

@lsout = `ls -RF /music/`;

for ( @lsout )
{
	chop;
	if ( m#.*:$# )				# we only want things ending with colons
								# (which are directories with contents)
	{
		chop;					# hack off the colon
		$path = $_;				# save what we have (path to playlist)
		s#/music/##;			# now get rid of the /root/music part
		s/_/ /g;				# replace underscores with spaces
		next if not m#/#;		# we don't want any top level directories
		s#/# - #;				# change the slash to a dash

		# okay, now some various exceptions and other little things to
		# pretty things up

		s/Various Artists - //;

		#omitted albums
		next if m/^Miscellaneous$/;
		$_ = "USAD - Opera CD" if $_ eq "USAD";
		$_ = "Christmas classics" if $_ eq "Christmas";

		push @albums, "$_\n";
		push @actions, $path;
	}
}

$i = 0;
for ( @albums )
{
	print;
	print "$actions[$i++]/album.m3u\n";
}
