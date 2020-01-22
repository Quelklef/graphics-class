



  
  numframes = 100 ;

  fnum = 0 ;
  
  while (1) {

    t = 1.0*fnum/numframes ; // goes from 0 to 1

    eye[0] = 15*cos(2*M_PI*t) ; 
    eye[1] =  6*t ; 
    eye[2] =  7*sin(2*M_PI*t) ; 

    // printf("t = %lf   eye = %lf %lf %lf\n",t, eye[0],eye[1],eye[2]) ;

    coi[0] =  0 ;
    coi[1] =  0 ; 
    coi[2] =  0 ;

    up[0]  = eye[0] ; 
    up[1]  = eye[1] + 1 ;
    up[2]  = eye[2] ; 


    D3d_view (V, Vi,  eye,coi,up) ;


    // move ALL objects from WORLD SPACE into EYE SPACE :
    for (onum = 0 ; onum < numobjects ; onum++) {
      D3d_mat_mult_points (x[onum],y[onum],z[onum],  V, 
                           x[onum],y[onum],z[onum],numpoints[onum]) ;
    }

    G_rgb(0,0,0) ; 
    G_clear() ;
    draw_all_objects() ;
    q = G_wait_key() ;  if (q == 'q') { break ; }

    // why are we doing this ? (important)
    for (onum = 0 ; onum < numobjects ; onum++) {
      D3d_mat_mult_points (x[onum],y[onum],z[onum],  Vi, 
                           x[onum],y[onum],z[onum],numpoints[onum]) ;
    }


    fnum++ ;
  } // end while (fnum ...
