/***************************************************************************
 *   videocapture Version 0.1                                              *
 *   Copyright (C) 2011 by Jan Michalowsky                                 *
 *   sejamich@googlemail.com                                               *
 *                                                                         *
 *   based on V4L2 Specification, Appendix B: Video Capture Example        *
 *   (http://v4l2spec.bytesex.org/spec/capture-example.html)               *
 *   contributor: Tobias Müller Tobias_Mueller@twam.info v4l2grab          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include "window.h"
#include "video.h"

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    setup_video_interface();
    
	create_main_window();
	printf("Hallo video projekt\n");  
    
	gtk_main();

    cleanup_video_interface();
	return 0;
}
