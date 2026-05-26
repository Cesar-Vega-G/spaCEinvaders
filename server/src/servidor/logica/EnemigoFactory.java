package servidor.logica;

import servidor.logica.modelo.Enemigo;

/**
 * PATRÓN FACTORY — Centraliza la creación de enemigos.
 * En vez de usar 'new Enemigo(...)' directamente, toda creación
 * pasa por esta fábrica. Facilita agregar nuevos tipos sin modificar
 * el resto del código.
 */
public class EnemigoFactory {

    /**
     * Crea un enemigo según su tipo string.
     * Ejemplo: EnemigoFactory.crear("CALAMAR", 100, 200, 0, 0)
     */
    public static Enemigo crear(String tipo, int x, int y, int fila, int columna) {
        switch (tipo.toUpperCase()) {
            case "CALAMAR":
                return new Enemigo(x, y, Enemigo.Tipo.CALAMAR, fila, columna);
            case "CANGREJO":
                return new Enemigo(x, y, Enemigo.Tipo.CANGREJO, fila, columna);
            case "PULPO":
                return new Enemigo(x, y, Enemigo.Tipo.PULPO, fila, columna);
            default:
                throw new IllegalArgumentException("Tipo de enemigo desconocido: " + tipo);
        }
    }

    /**
     * Crea un enemigo según su fila en la formación clásica.
     * Fila 0 = CALAMAR, Fila 1 = CANGREJO, Fila 2 = PULPO
     */
    public static Enemigo crearPorFila(int fila, int x, int y, int columna) {
        // Distribución clásica de Space Invaders: 5 filas
        // Fila 0 → CALAMAR  (rojo,  10pts)  — 1 fila
        // Fila 1 → CANGREJO (verde, 20pts)  — 2 filas
        // Fila 2 → CANGREJO (verde, 20pts)
        // Fila 3 → PULPO    (azul,  40pts)  — 2 filas
        // Fila 4 → PULPO    (azul,  40pts)
        Enemigo.Tipo[] tipos = {
            Enemigo.Tipo.CALAMAR,
            Enemigo.Tipo.CANGREJO,
            Enemigo.Tipo.CANGREJO,
            Enemigo.Tipo.PULPO,
            Enemigo.Tipo.PULPO
        };
        if (fila < 0 || fila >= tipos.length) {
            throw new IllegalArgumentException("Fila inválida: " + fila);
        }
        return new Enemigo(x, y, tipos[fila], fila, columna);
    }
}