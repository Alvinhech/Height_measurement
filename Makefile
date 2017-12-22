test:
	make -f ./make_dir/makefile_run
	make -f ./make_dir/makefile_radar_no
	make -f ./make_dir/makefile_cal_radar
	make -f ./make_dir/makefile_process

clean:
	make clean -f ./make_dir/makefile_run
	make clean -f ./make_dir/makefile_radar_no
	make clean -f ./make_dir/makefile_cal_radar
	make clean -f ./make_dir/makefile_process
