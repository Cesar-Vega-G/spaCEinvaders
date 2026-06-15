package servidor.logica.modelo;

import java.util.Random;

/**
 * Modelo del OVNI que atraviesa la pantalla por la parte superior.
 *
 * El OVNI aparece automáticamente cada INTERVALO_OVNI frames (definido en
 * EstadoJuego) o al ejecutar el comando de admin "OVNI dir pts".
 * Cruza de un borde al otro y desaparece sin disparar.
 * Otorga puntos variables (50–300) como bonus por reacción rápida.
 */
public class Ovni {

    public static final int ANCHO  = 60;
    public static final int ALTO   = 30;
    public static final int Y_FIJO = 30; /* siempre en la franja superior */

    private static final Random random = new Random();

    private int     x, y;
    private int     velocidad;
    private int     direccion; /* 1 = izquierda→derecha, -1 = derecha→izquierda */
    private int     puntos;
    private boolean activo;

    public Ovni() {
        this.y      = Y_FIJO;
        this.activo = false;
    }

    /**
     * Activa el OVNI haciéndolo aparecer por el borde correspondiente a
     * la dirección.  Si puntos <= 0, se asignan puntos aleatorios (50–300).
     */
    public void aparecer(int anchoPantalla, int direccion, int puntos) {
        this.direccion = direccion;
        this.puntos    = puntos > 0 ? puntos : (random.nextInt(6) + 1) * 50;
        this.velocidad = 6;
        this.activo    = true;
        /* Aparece justo fuera del borde para que el jugador no lo vea "aparecer". */
        this.x = direccion == 1 ? -ANCHO : anchoPantalla;
    }

    /* Avanza horizontalmente y se desactiva al salir por el borde opuesto. */
    public void actualizar(int anchoPantalla) {
        if (!activo) return;
        x += velocidad * direccion;
        if (x + ANCHO < 0 || x > anchoPantalla) activo = false;
    }

    public void destruir() { activo = false; }

    /* Colisión AABB con el rect del proyectil (bx,by,bw,bh). */
    public boolean colisionaCon(int bx, int by, int bw, int bh) {
        return activo &&
               bx < x + ANCHO && bx + bw > x &&
               by < y + ALTO  && by + bh  > y;
    }

    public int     getX()       { return x;      }
    public int     getY()       { return y;      }
    public boolean isActivo()   { return activo; }
    public int     getPuntos()  { return puntos; }

    /* Formato: "OVNI x y 0|1 puntos" */
    public String serializar() {
        return String.format("OVNI %d %d %d %d", x, y, activo ? 1 : 0, puntos);
    }
}
