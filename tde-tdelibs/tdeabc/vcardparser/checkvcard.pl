#!/usr/bin/perl

use utf8;
use strict;
use warnings;

if ( @ARGV != 1 ) {
  print STDERR "Missing arg: filename\n";
  exit 1;
}

my $file = $ARGV[0];
my $ref = "$file.ref";

my $options="";
my $error=0;
my @prscont;
my @refcont;

open( IN, "<", $file ) || die ("Unable to open $file");
while( <IN> ) {
  if (/^VERSION:(.*)$/ ) { my $v = $1; $options = "--vcard21" if $v eq "2.1"; }
}
close IN;

open( REF, "$ref" ) ||  die ("Unable to open $ref");
while( <REF> ) {
  next if $_ =~ /^UID/;
  push @refcont , $_ ;
}
close REF;

open( READ, "./testread $file $options 2> /dev/null |" ) || die  ("Unable to open testread");
print "Checking: $file ";
while( <READ> ) {
  next if $_ =~ /^UID/;
  push @prscont , $_;
}
close READ;


if ( $#refcont != $#prscont ) {
  print "\n  FAILED: ref size and parsed size mismatch.\n";
  system "touch FAILED";
  exit 1;
}
@prscont = sort @prscont;
@refcont = sort @refcont;
for (my $i=0; $i<=$#refcont; $i++) {
   if ( $refcont[$i] ne $prscont[$i] ) {
	$error++;
	print "\n  Expected      : $refcont[$i]";
	print "  Parser output : $prscont[$i]";
   }
}

if ( $error > 0 ) {
  print "\n  FAILED: $error errors found.\n";
  system "touch FAILED";
  exit 1;
} else {
  print "  OK\n";
}

exit 0;
