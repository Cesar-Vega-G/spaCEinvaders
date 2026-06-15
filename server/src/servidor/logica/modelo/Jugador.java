package servidor.logica.modelo;

/**
 * Modelo del jugador en el servidor.
 *
 * El servidor es dueño de toda la lógica: posición, vidas y puntaje
 * solo se modifican aquí y se serializan hacia los clientes cada frame.
 */
public class Jugador {

    public static final int ANCHO           = 65;
    public static final int ALTO            = 30;
    public static final int VELOCIDAD       = 35;  /* píxeles por comando de movimiento */
    public static final int INICIO_X        = 380;
    public static final int INICIO_Y        = 810;
    public static final int VIDAS_INICIALES = 3;

    private int id;
    private int x;
    private int y;
    private int vidas;
    private int puntaje;

    public Jugador(int id) {
        this.id      = id;
        /* El jugador 1 aparece 300 px a la derecha del jugador 0. */
        this.x       = INICIO_X + (id * 300);
        this.y       = INICIO_Y;
        this.vidas   = VIDAS_INICIALES;
        this.puntaje = 0;
    }

    /* Mueve el jugador a la izquierda sin salir de la pantalla. */
    public void moverIzquierda() {
        if (x > 0) x -= VELOCIDAD;
    }

    /* Mueve el jugador a la derecha sin salir de la pantalla. */
    public void moverDerecha(int anchoPantalla) {
        if (x + ANCHO < anchoPantalla) x += VELOCIDAD;
    }

    public void perderVida()           { vidas--;        }
    public void ganarVida()            { vidas++;        }
    public void sumarPuntaje(int pts)  { puntaje += pts; }
    public boolean estaVivo()          { return vidas > 0; }

    public int getId()      { return id;      }
    public int getX()       { return x;       }
    public int getY()       { return y;       }
    public int getVidas()   { return vidas;   }
    public int getPuntaje() { return puntaje; }

    /* Formato: "JUGADOR id x y vidas puntaje" */
    public String serializar() {
        return String.format("JUGADOR %d %d %d %d %d", id, x, y, vidas, puntaje);
    }
}
