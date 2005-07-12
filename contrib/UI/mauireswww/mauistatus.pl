#!/usr/bin/perl
#
# Version: 1.3
#
# Copyright (C) 2001 Niclas Andersson
# National Supercomputer Centre, Sweden
#

# NOTE:  script must be configured by setting the following variables

$MAUIPATH="/usr/local/maui/bin";
$CGIPATH="/home/www/bin";
$MACHINENAME="Ingvar";

use CGI qw/:standard/;

$refresh = 900;

$bggrid5 = "bggrid5.gif"; # URL

$style = <<EOF;
p,th,td { font-family: Arial,Helvetica; }
/* mauireswww */
.nodename { font-size: 12px; text-align: right; }
.nodealloc td { font-size: 12px}
.nodealloc th { font-style: bold; text-align: left; background: url($bggrid5); background-color: #ffffa0; }
.reservations { fontsize: 1px; white-space: pre; background: url($bggrid5); background-color: #ffffa0; }
.legend { font-size: 14px}
.note { color: #800000 }
EOF

# newline
sub nl() { return "\n"; }

$date = localtime();

print header(-expires=>'+1m',
	     -Refresh=>"$refresh; URL=" . url());
print start_html(-title=>'${MACHINENAME} Status',
		 -bgcolor=>'#ffffff',
		 -head=>style($style));
print 
    h1('${MACHINENAME} Status'), nl,
    h4($date), nl, hr({-noshade});

#--------------------------------------------------------------------------

print h3("Node Reservations:");

open(NODERES, "${MAUIPATH}/showres -n | ${CGIPATH}/mauireswww |") 
  or die;
@restbl = <NODERES>;
close(NODERES);
print @restbl;

print p, <<EOF;
The priority of idle jobs, whether their projects have normal
or bonus priority is not yet shown.
Normal priority jobs are always scheduled before idle, not started, 
bonus priority jobs.

<p><b class="note">Note:</b> 
All advance job reservation is target for rescheduling
every scheduling iteration (60 seconds) and whenever new jobs 
enter the batch system.

<p><a href="mauistatus.html">Description of the graph</a>
EOF

#--------------------------------------------------------------------------

print hr({-noshade}), nl, h3("Job Status:");
open(SHOWQ, "${MAUIPATH}/showq |") or die;
while (<SHOWQ>) {
    if (/ACTIVE JOBS/) { 
	print h4("Active Jobs"), "<pre>"; 
	next; 
    }
    if (/IDLE JOBS/) { 
	print "</pre>", h4("Idle Jobs"), "<pre>"; 
	next; 
    }
    if (/NON-QUEUED JOBS/) { 
	print "</pre>", h4("Non-Queued Jobs"), "<pre>"; 
	next; 
    }
    if (/JOBNAME/) { 
	print b($_); 
	next;
    }
    print;
}
print "</PRE>";
close(SHOWQ);

#--------------------------------------------------------------------------

print hr({-noshade}), nl,
    address(a({-href=>'/~nican/'}, "Niclas Andersson")),
    end_html, nl;

