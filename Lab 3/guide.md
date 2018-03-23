## Finding type in which UNIX time is stored.
1. time.c assigns time to mp->mp_reply.reply_utime.
2. reply_utime is defined as m2_l2 within param.h.
3. reply_utime is defined as m_u.m_m2.m2l2 in ipc.h, which in turn also defines union m_u, and m_m2, and finally m2l2 as a long within struct mess_2. 
4. So reply_utime is a long. Good to know. 

## Finding object in which UNIX time is stored (Location of mp referenced in time.c)
1. It would appear appear "mp" has several definitions. It could be almost anything. One declaration was found to be: `/usr/src/./sys/ufs/ufs/ufs_vfsops.c:108:	(void) mp;`. This seems to generally describe a pointer to a message (I.E: "mp ~ message pointer" but I'm not certain).
2. A better objective is to just import the right files so that you can reference it like `do_time` in `time.c` does. I found that the declaration for `do_time` is located in `proto.h`. 
3. The file gettimeofday.c is what I'm currently trying to extract time with.


## Writing the system call.
1. The regular instructions were followed for making the do_utctime system call. It's located in `/usr/src/servers/pm` and called `do_utctime.c`. However it doesn't return anything. I need to extract the time from the message and return that to the user.
2. I need to define a library wrapper. I defined `time_t utctime(time_t *)` in `time.h` within `/usr/src/include` right after the `time_t time(time_t *)` declaration.
3. I wrote the library function (`utctime.c`) and copied it into `/usr/src/lib/libc/sys-minix`.
4. I added the new C file to the `Makefile.inc`  within the same directory. (Recall I don't need to add any header file because I declared this in `time.h` earlier which should be already in the make files.




