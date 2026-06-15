package servidor.logica.modelo;

/**
 * Modelo de la bala disparada por un jugador.
 *
 * La bala sube VELOCIDAD píxeles por frame. Se desactiva automáticamente
 * cuando sale de la pantalla por arriba (y + ALTO < 0).
 * Solo puede haber una bala activa por jugador a la vez.
 */
public class Bala {

    public static final int ANCHO     = 5;
    public static final int ALTO      = 15;
    public static final int VELOCIDAD = 25; /* píxeles por frame hacia arriba */

    private int x, y;
    private boolean activa;
    private int idJugador;

    public Bala(int idJugador) {
        this.idJugador = idJugador;
        this.activa    = false;
    }

    /* Activa la bala en la posición dada; si ya está activa, ignora el disparo. */
    public void disparar(int x, int y) {
        if (!activa) {
            this.x = x;
            this.y = y;
            this.activa = true;
        }
    }

    /* Avanza la bala hacia arriba y la desactiva al salir de la pantalla. */
    public void actualizar() {
        if (activa) {
            y -= VELOCIDAD;
            if (y + ALTO < 0) activa = false;
        }
    }

    public void desactivar()   { activa = false; }
    public int getX()          { return x;          }
    public int getY()          { return y;          }
    public boolean isActiva()  { return activa;     }
    public int getIdJugador()  { return idJugador;  }

    /* Formato: "BALA idJugador x y 0|1" */
    public String serializar() {
        return String.format("BALA %d %d %d %d", idJugador, x, y, activa ? 1 : 0);
    }
}
