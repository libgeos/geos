deb:
	ln -sf libgeos-dev.install libgeos-dev.files
	ln -sf libgeos.install libgeos.files
	cd .. && fakeroot debian/rules binary 

clean:
	rm -f libgeos-dev.files
	rm -f libgeos.files
	cd .. && fakeroot debian/rules clean
