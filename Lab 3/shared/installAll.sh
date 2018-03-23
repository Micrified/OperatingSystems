# Installation Script for Operating Systems Lab 3.
# Authors:  Barnabas Busa, Charles Randolph, Joe Jones.
# SNumbers: s2922673, s2897318, s2990652
# Requirements: The "shared" folder and its contents must be mounted as: /mnt/shared

# INSTALLATION: Startup Banner.
echo ++ Modifying Minix Startup Banner \(Requires files be located /mnt/shared\).

# 1. Overwriting main.c at /usr/src/kernel/main.c
cp /mnt/shared/main.c /usr/src/kernel/

echo Done.

# INSTALLATION: System Call.
echo ++ Installing UTC System Call \(Requires files be located /mnt/shared\).
# 1. Adding entry to /usr/src/servers/pm/table.c

cp /mnt/shared/table.c /usr/src/servers/pm/

# 2. Adding system call number definition /usr/src/include/minix/callnr.h

cp /mnt/shared/callnr.h /usr/src/include/minix

# 3. Appending system call header to /usr/src/servers/pm/proto.h

cp /mnt/shared/proto.h /usr/src/servers/pm/

# 4. Copying system call file do_utctime.c to /usr/src/servers/pm

cp /mnt/shared/do_utctime.c /usr/src/servers/pm/

# 5. Modifying the Makefile to include do_utctime.c

cp /mnt/shared/Makefile /usr/src/servers/pm/

echo Done.

# INSTALLATION: Library Call.
echo ++ Installing Library Wrapper.

# 1. Installing library routine header within time.h /usr/src/include/
cp /mnt/shared/time.h /usr/src/include/

# 2. Copying utctime.c to directory /usr/src/lib/libc/sys-minix/
cp /mnt/shared/utctime.c /usr/src/lib/libc/sys-minix/

# 3. Adding utctime.c to the Makefile.inc file the same directory.
cp /mnt/shared/Makefile.inc /usr/src/lib/libc/sys-minix/

echo Done. 

# Copy test program to root.
cp /mnt/shared/utc_test.c /root/

echo All tasks done. 
echo Recompile kernel to see change. Root has example program \(utc_test.c\) provided. 
