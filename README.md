# Tuki3-3d
Tuki3 3d è uno spinoff del progetto [Tuki e Giuli](https://github.com/francescosisini/LIBRO-Sfidare-gli-algoritmi-5-videogiochi-in-C-su-Linux-codice) che 
realizza  una interfaccia 3d con openGL per Tuki3

L'idea è nata per il progetto di Grafica Computerizzata di [Valentina](https://github.com/ValentinaSisini) che prevedeva lo sviluppo di una applicazione 3d implementata con la libreria Open Source per la renderizzazione in 3d OpenGL

Per provare il gioco:
- scaricare e installare OpenGL e Glew per Linux
- compilare Tuki3_3d con:
```gcc -o tuki3.play tuki3_mvc.c turno_tuki.c view3d.c Utils.c -lglut -lGL -lGLU -lGLEW -lm```

![Tuki3_3d](Tuki3_3d.png)
