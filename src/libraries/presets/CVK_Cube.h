#ifndef __CVK_CUBE_H
#define __CVK_CUBE_H

#include "CVK_Geometry.h"


/**
* The cube is one of the example geometry classes in the CVK.
* @brief Cube class as Geometry
*/
class Cube : public Geometry
{
public:
	/**
	 * Standard Constructor for Cube
	 */
	Cube();
	/**
	 * Constructor for Cube with given size
	 */
	Cube(float size);
	/**
	 * Standard Destructor for Cube
	 */
	~Cube();
	
	/**
	 * @brief Standard Setter for size
	 * @param size the new size of this object
	 */
	void setSize(float size);

protected:
	/**
	 * Create a Cube and the buffers with the given attributes
	 * @brief Create the Cube and the buffers
	 * @param size The size of each of the sides
	 */
	void create(float size);

	float m_size;
};

#endif /* __CVK_CUBE_H */
