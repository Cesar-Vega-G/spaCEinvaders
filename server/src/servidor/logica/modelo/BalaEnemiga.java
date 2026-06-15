package servidor.logica.modelo;

/**
 * Bala disparada por un enemigo. Se mueve hacia ABAJO (y aumenta).
 *
 * El servidor mantiene hasta MAX_BALAS_ENEMIGAS instancias en memoria.
 * Cuando un enemigo dispara, se busca la primera inactiva y se activa
 * llamando a disparar().  El pool evita crear y destruir objetos cada frame.
 *
 * A diferencia de Bala (jugador), no hay restricción de "una por tirador":
 * cualquier enemigo puede disparar mientras haya un slot libre.
 */
public class BalaEnemiga {

    public static final int ANCHO     = 5;
    public static final int ALTO      = 15;
    public static final int VELOCIDAD = 10; /* píxeles por frame hacia abajo */

    private int     id;     /* 0..MAX-1, índice estable para serialización */
    private int     x, y;
    private boolean activa;

    public BalaEnemiga(int id) {
        this.id     = id;
        this.activa = false;
    }

    /* Activa la bala en la posición dada (si no está ya activa). */
    public void disparar(int x, int y) {
        if (!activa) {
            this.x = x;
            this.y = y;
            this.activa = true;
        }
    }

    /* Avanza la bala hacia abajo; la desactiva al salir por el borde inferior. */
    public void actualizar(int altoPantalla) {
        if (!activa) return;
        y += VELOCIDAD;
        if (y > altoPantalla) activa = false;
    }

    public void desactivar() { activa = false; }

    /* Detección de colisión AABB con cualquier rectángulo (rx,ry,rw,rh). */
    public boolean colisionaCon(int rx, int ry, int rw, int rh) {
        return activa &&
               x < rx + rw && x + ANCHO > rx &&
               y < ry + rh && y + ALTO  > ry;
    }

    public int     getId()        { return id;    }
    public int     getX()         { return x;     }
    public int     getY()         { return y;     }
    public boolean isActiva()     { return activa; }

    /* Formato: "BALA_ENEMIGA id x y 0|1" */
    public String serializar() {
        return String.format("BALA_ENEMIGA %d %d %d %d", id, x, y, activa ? 1 : 0);
    }
}
