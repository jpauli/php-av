This is a libav wrapper for PHP.
It may replace ffmpeg.

* PHP >= 5.3
* Not tested under Windows, might work

Install
=======

    > phpize
    > ./configure
    > make
    > make install
    
Example
=======

    <?php
        $r = av_file_open("/tmp/my_vid.avi");
        var_dump(av_file_get_metadata($r));
        /*array(4) {
        ["nb_streams"]=>
        int(2)
        ["filename"]=>
        string(12) "/tmp/vid.avi"
        ["duration"]=>
        int(171666667)
        ["bit_rate"]=>
        int(1437242)
        }*/
        
        var_dump(av_libav_get_info());
        /*
        array(3) {
        ["version"]=>
        string(7) "53.21.1"
        ["configuration"]=>
        string(678) "--arch=amd64 --enable-pthreads --enable-runtime-cpudetect --extra-version='6:0.8.10-1' .....
        ["license"]=>
        string(22) "GPL version 2 or later"
        }
        */

        av_libav_file_close($r);

Wip
===

Sure, WIP.
In the end, you should be able to do what libav can do, in PHP.
Conversions, resampling etc.. Consult libav manual for features http://libav.org/documentation.html.

Bridges to ext/gd may also be developed.
Bridges with PHP stream layer may as well be developed in future.