
HEADERS	= ../sifs.h sifs-internal.h md5.h unit_test.h
LIBRARY	= libsifs.a

OBJECTS	= mkvolume.o helper.o mkdir.o\
	 	rmdir.o dirinfo.o writefile.o readfile.o\
		rmfile.o fileinfo.o perror.o md5.o unit_test.o

CC      = cc
CFLAGS  = -std=c99 -Wall -Werror -pedantic -g

$(LIBRARY):	$(OBJECTS)
	ar rc $(LIBRARY) $(OBJECTS)
	ranlib $(LIBRARY)
	cp $(LIBRARY) ..


%.o:	%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<


# ----------------------------------------------------------------

clean:
	rm -f $(LIBRARY) $(OBJECTS)
