#! perl -w

use strict;
use warnings;

use FindBin;

use Getopt::Long qw(:config bundling);
# use Pod::Usage;

sub slurp
{
	my $fn = $_[0];
	my $fh;
	open( $fh, "<", $fn ) or die "Can't open $fn for reading: $!";
	local $/;
	my $file_content = <$fh>;
	close( $fh );
	chomp $file_content;
	return $file_content;
}

my %cmdOptions = ();
GetOptions( \%cmdOptions,
	    # "help|h" => sub { pod2usage(0); },
	    "input-file|i=s" =>,
	    #"output-file|o=s" =>,
	    "defines|d=s%" =>,
	    "file-contents|f=s%" =>,
	  ) or exit(1);

defined( $cmdOptions{"input-file"} ) or die "Missing input-file";
-r $cmdOptions{"input-file"} or die $cmdOptions{"input-file"} . ": $!";

my $src = slurp($cmdOptions{"input-file"});

if( defined( $cmdOptions{"file-contents"} ) and "HASH" eq ref($cmdOptions{"file-contents"}) )
{
	my %contents = map { $_ => slurp($cmdOptions{"file-contents"}->{$_}) } keys %{$cmdOptions{"file-contents"}};
	my $cntkeys = join( "|", keys %contents );

	$src =~ s/@($cntkeys)[@]/$contents{$1}/ge;
}

if( defined( $cmdOptions{"defines"} ) and "HASH" eq ref($cmdOptions{"defines"}) )
{
	my %defines = %{$cmdOptions{"defines"}};
	my $defkeys = join( "|", keys %defines );

	$src =~ s/@($defkeys)[@]/$defines{$1}/ge;
}

unless( $src =~ m/^#!.*perl/ )
{
	$src = "#!$^X\n\n$src";
}

print "$src\n";
