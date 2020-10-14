# MASS C++ Software License
# © 2014-2015 University of Washington
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# The following acknowledgment shall be used where appropriate in publications, presentations, etc.:
# © 2014-2015 University of Washington. MASS was developed by Computing and Software Systems at University of Washington Bothell.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

#/bin/bash
######################################################################
#
#       Kill MASS.MProcss for user
#       ---------------------------
#       Just a small script to KILL all MASS.MProcess process for the
#	user on each of the machines listed in the machinefile.txt file
#
#       By:     Richard Romanus
#       Date:   07/01/2013
#
######################################################################
filename="./machinefile.txt"

for host in `cat ${filename}`
do
	echo -e "\n${host}"
	echo -e   "============="
	tmp=`ssh ${USER}@${host} 'ps -ef  | grep "${USER}"
			' | grep "mprocess" | grep -v "grep"` 
	echo -e ">>> $tmp"

	for pNum in `echo $tmp | awk '{print $2}' `
	do
		echo -e "> kill ${pNum}" 
		ssh ${USER}@${host} "kill ${pNum}"
		echo -e "  -- DONE! \n"
	done
done

echo -e "\n"
