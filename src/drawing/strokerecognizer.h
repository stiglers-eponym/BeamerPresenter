#ifndef STROKERECOGNIZER_H
#define STROKERECOGNIZER_H

#include "src/drawing/basicgraphicspath.h"

/**
 * @brief StrokeRecognizer: use only temporarily to analyze a path
 *
 * Objects of this class should only briefly be created for a given path and
 * must be deleted before deleting the path.
 *
 * @todo continue implementing this class
 * @todo implement simpler way of accessing stroke recognizer
 */
class StrokeRecognizer
{
    /// Stroke which should be recognized. The stroke recognizer does not
    /// own this stroke. It must not be deleted while the StrokeRecognizer
    /// is in use.
    const AbstractGraphicsPath *stroke;

    double  s = 0., ///< sum of weights (pressures)
            sx = 0., ///< weighted sum of x coordinates
            sy = 0., ///< weighted sum of y coordinates
            sxx = 0., ///< weighted sum of x*x
            sxy = 0., ///< weighted sum of x*y
            syy = 0., ///< weighted sum of y*y
            sxxx = 0., ///< weighted sum of x*x*x
            sxxy = 0., ///< weighted sum of x*x*y
            sxyy = 0., ///< weighted sum of x*y*y
            syyy = 0., ///< weighted sum of y*y*y
            sxxxx = 0., ///< weighted sum of x*x*x*x
            sxxyy = 0., ///< weighted sum of x*x*y*y
            syyyy = 0.; ///< weighted sum of y*y*y*y

    /**
     * Loss function for ellipse fit.
     * @param mx x coordinate of center
     * @param my y coordinate of center
     * @param ax = 1/(rx*rx) = inverse radius in x-direction squared
     * @param ay = 1/(ry*ry) = inverse radius in y-direction squared
     *
     * The parameters ax = 1/(rx*rx) and ay = 1/(ry*ry) are used instead of rx
     * and ry to make this function a polynomial.
     *
     * This returns
     * \[ \sum_k w_k \left[ \frac{(x_k-m_x)^2}{rx^2} + \frac{(y_k-m_y)^2}{ry^2} - 1 \right]^2 \]
     * where (x_k, y_k) is a coordinate with weight w_k.
     */
    qreal ellipseLossFunc(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                s*bc*bc
                + sxxxx*ax*ax
                + syyyy*ay*ay
                + 2*sxxyy*ax*ay
                + 2*(2*mx*mx*ax + bc)*sxx*ax
                + 2*(2*my*my*ay + bc)*syy*ay
                + 8*mx*my*sxy*ax*ay
                - 4*mx*ax*(sxxx*ax + sxyy*ay)
                - 4*my*ay*(sxxy*ax + syyy*ay)
                - 4*bc*(mx*sx*ax + my*sy*ay);
    }
    /**
     * First derivative of ellipseLossFunc by mx.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_mx(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                4*s*bc*mx*ax
                + 12*mx*sxx*ax*ax
                + 4*mx*syy*ax*ay
                + 8*my*sxy*ax*ay
                - 4*ax*(sxxx*ax + sxyy*ay)
                - 4*bc*sx*ax
                - 8*mx*ax*(mx*sx*ax + my*sy*ay);
    }
    /**
     * First derivative of ellipseLossFunc by my.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_my(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                4*s*bc*my*ay
                + 12*my*syy*ay*ay
                + 4*my*sxx*ax*ay
                + 8*mx*sxy*ax*ay
                - 4*ay*(sxxy*ax + syyy*ay)
                - 4*bc*sy*ay
                - 8*my*ay*(mx*sx*ax + my*sy*ay);
    }
    /**
     * First derivative of ellipseLossFunc by ax.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_ax(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                2*s*mx*mx*bc
                + 2*sxxxx*ax
                + 2*sxxyy*ay
                + 2*(5*mx*mx*ax + bc)*sxx
                + 2*mx*mx*syy*ay
                + 8*mx*my*sxy*ay
                - 4*mx*(2*sxxx*ax + sxyy*ay)
                - 4*my*ay*sxxy
                - 4*mx*mx*(mx*sx*ax + my*sy*ay)
                - 4*bc*mx*sx;
    }
    /**
     * First derivative of ellipseLossFunc by ay.
     * @see ellipseLossFunc
     */
    qreal ellipseLossGradient_ay(const qreal mx, const qreal my, const qreal ax, const qreal ay) const noexcept
    {
        const qreal bc = mx*mx*ax + my*my*ay - 1;
        return
                2*s*my*my*bc
                + 2*syyyy*ay
                + 2*sxxyy*ax
                + 2*(5*my*my*ay + bc)*syy
                + 2*my*my*sxx*ax
                + 8*mx*my*sxy*ax
                - 4*my*(sxxy*ax + 2*syyy*ay)
                - 4*mx*ax*sxyy
                - 4*my*my*(mx*sx*ax + my*sy*ay)
                - 4*bc*my*sy;
    }

public:
    /// Trivial constructor.
    StrokeRecognizer(const AbstractGraphicsPath *stroke) : stroke(stroke) {}

    /// Trivial destructor.
    ~StrokeRecognizer() {}

    /// Compute statistical moments of points in stroke up to second moment.
    void calc1() noexcept;
    /// Compute third and fourth statistical moments of points in stroke up.
    void calc2() noexcept;

    /// Try to recognize a known shape in stroke.
    /// Return NULL if no shape was detected.
    BasicGraphicsPath *recognize();

    /// Check if stroke is a line.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    BasicGraphicsPath *recognizeLine() const;

    /// Check if stroke is a rectangle.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    /// @todo not implemented yet
    BasicGraphicsPath *recognizeRect() const;

    /// Check if stroke is an ellipse.
    /// Return a BasicGraphicsPath* representing this line if successful, NULL otherwise.
    /// @todo not implemented yet
    BasicGraphicsPath *recognizeEllipse() const;
};

#endif // STROKERECOGNIZER_H
