MSR605
======

A C/CPP Library for interacting with the the MSR605 (and other MSR206 compatible) Magstripe Reader/Writers.

Example
=======
<pre>
$ ./msr605 8 8 8 1
MSR605  Copyright (C) 2013 Pentura 
This program comes with ABSOLUTELY NO WARRANTY; 
This is free software, and you are welcome to redistribute it under certain conditions;

Connected to /dev/ttyUSB0
Firmware: REVU2.31
Model: 3
Comm Test Sent...
Receiving Response.
Initialized MSR605.
Waiting for swipe...
Track 1: 
Track 2: 1042c16f1de7f000
Track 3: 
</pre>

License
=======
MSR605 / Libmsr605 by Andy Davies
Copyright (C) 2013  Pentura

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
