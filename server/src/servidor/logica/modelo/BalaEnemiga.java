package servidor.logica.modelo;

/**
 * Bala disparada por un enemigo. Se mueve hacia ABAJO.
 * Misma idea que Bala pero con velocidad invertida y sin idJugador.
 */
public class BalaEnemiga {

    public static final int ANCHO     = 5;
    public static final int ALTO      = 15;
    public static final int VELOCIDAD = 10;

    private int id;          // 0..MAX-1, para serializar de forma estable
    private int x, y;
    private boolean activa;

    public BalaEnemiga(int id) {
        this.id     = id;
        this.activa = false;
    }

    public void disparar(int x, int y) {
        if (!activa) {
            this.x = x;
            this.y = y;
            this.activa = true;
        }
    }

    public void actualizar(int altoPantalla) {
        if (!activa) return;
        y += VELOCIDAD;
        if (y > altoPantalla) activa = false;
    }

    public void desactivar() { activa = false; }

    public boolean colisionaCon(int rx, int ry, int rw, int rh) {
        return activa &&
               x < rx + rw && x + ANCHO > rx &&
               y < ry + rh && y + ALTO  > ry;
    }

    public int getId()         { return id; }
    public int getX()          { return x; }
    public int getY()          { return y; }
    public boolean isActiva()  { return activa; }

    public String serializar() {
        return String.format("BALA_ENEMIGA %d %d %d %d", id, x, y, activa ? 1 : 0);
    }
}
