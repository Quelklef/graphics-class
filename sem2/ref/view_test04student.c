



  if (choice == 0) {

    D3d_make_identity (V) ;  D3d_make_identity (Vi) ;  D3d_translate(V,Vi, 0,0,5) ;

  } else {

    double eye[3], coi[3], up[3] ;
    eye[0] = 15 ; eye[1] =  5 ; eye[2] =  5 ;
    coi[0] =  0 ; coi[1] =  0 ; coi[2] =  4 ;
    up[0]  = eye[0] ; 
    up[1]  = eye[1] + 1 ;
    up[2]  = eye[2] ; 
    D3d_view (V, Vi,  eye,coi,up) ;

  }

