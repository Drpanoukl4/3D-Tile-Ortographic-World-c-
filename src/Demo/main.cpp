
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "World.h"

class Shadow : public olc::PixelGameEngine
{

public:

    struct sCell 
    {

        bool bWall = false;
        bool bExist = false;
        int cHeight = 20;
        olc::vi2d id[6]{  };

    };

    struct sQuad 
    {

        vec3D points[4];
        olc::vf2d tile;

    };

    bool Wireframe = 0;

    //Camera//

    olc::vf2d Camera = { 0.0f, 0.0f };
    float fCameraAngle = 0.0f;
    float fCameraPitch = 5.0f;
    float fCameraZoom = 16.0f;

    //float ftargetCameraPitch = 0.0f; // // xD
    float ftargetCameraAngle = 0.0f;

    vecint3D vCursor = {0, 0, 0};
    olc::vi2d sCursor = {0, 0};
    olc::vi2d eCamera;
    //Camera//

    struct Renderable
	{
		Renderable() {}

		void Load(const std::string& sFile)
		{
			sprite = new olc::Sprite(sFile);
			decal = new olc::Decal(sprite);
		}

		~Renderable()
		{
			delete decal;
			delete sprite;
		}

		olc::Sprite* sprite = nullptr;
		olc::Decal* decal = nullptr;
	};

    olc::vi2d TileSize = {32, 32};
    olc::vi2d vTileCursor = {0, 0};

    Renderable RenderAllWall;
    Renderable Select;

    // Backface Culling //
    bool bVisible[6];

    enum Face 
    {

        Floor = 0,
        North = 1,
        East = 2,
        South = 3,
        West = 4,
        Top = 5

    };

    class sWorld 
    {

    public:

        sWorld () 
        {

        

        }

        void Create ( int Height, int Width, int Depth ) 
        {

            size = { Width , Height, Depth };
            World.resize( Height * Width * Depth );

        }

        sCell& GetCell ( const vecint3D& v ) 
        {

            if( v.x >= 0.0f && v.x < size.x && v.y >= 0.0f && v.y < size.y )
            {

                return World[ (size.x * size.y * v.z) + (size.x * v.y) + v.x ];

            }else 
            {

                return NullCell;

            }

        }

    public:

        vecint3D size;

    private:

        std::vector< sCell > World;
        sCell NullCell;
    
    };

    sWorld mWorld;

public:
	Shadow()
	{
		sAppName = "Example";
	}

public:

    void CalculateVisibleFace ( std::array < vec3D, 8 > &Cube ) 
    {

        auto CheckNormals  = [&] ( int v1, int v2, int v3 ) 
        {

            olc::vf2d a = { Cube[v1].x, Cube[v1].y };
            olc::vf2d b = { Cube[v2].x, Cube[v2].y };
            olc::vf2d c = { Cube[v3].x, Cube[v3].y };

            olc::vf2d vAB = { b.x - a.x, b.y - a.y};
            olc::vf2d vCB = { c.x - a.x, c.y - a.y};

            float normal = (vAB.x * vCB.y) - (vAB.y * vCB.x);

            return normal > 0;

        };

        bVisible [ Face::Floor ] = CheckNormals( 4, 0, 1 );
        bVisible [ Face::South ] = CheckNormals( 3, 0, 1 );
        bVisible [ Face::North ] = CheckNormals( 6, 5, 4 );
        bVisible [ Face::East ] = CheckNormals( 7, 4, 0 );
        bVisible [ Face::West ] = CheckNormals( 2, 1, 5 );
        bVisible [ Face::Top ] = CheckNormals( 7, 3, 2 );

    }

    void getFaceQuad( const vecint3D  &vCell, const float fAngle, const float Pitch, const float fScale, const vec3D &Camera, std::vector < sQuad >& render, int Height = 0) 
    {

        auto &cell = mWorld.GetCell(vCell);

        std::array < vec3D, 8 > projCube = CreateCube( vCell, fAngle, Pitch, fScale, Camera, Height );

        auto MakeFaces = [&] ( int v1, int v2, int v3, int v4, Face f, std::array<vec3D, 8> &projCube )
        {

            render.push_back( { projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f] } );

        };

        if( !cell.bWall ) 
        {

            if(bVisible[ Face::Floor ]) MakeFaces( 4, 0, 1, 5, Face::Floor, projCube );

        }else 
        {

            if(bVisible[ Face::South ]) MakeFaces( 3, 0, 1, 2, Face::South, projCube );
            if(bVisible[ Face::North ]) MakeFaces( 6, 5, 4, 7, Face::North, projCube );
            if(bVisible[ Face::East ]) MakeFaces( 7, 4, 0, 3, Face::East, projCube );
            if(bVisible[ Face::West ]) MakeFaces( 2, 1, 5, 6, Face::West, projCube );
            if(bVisible[ Face::Top ]) MakeFaces( 7, 3, 2, 6, Face::Top, projCube );



        }

    }

    std::array < vec3D, 8 > CreateCube ( const vecint3D & vCell, const float fAngle, const float Pitch, const float fScale, const vec3D &Camera, int Height) 
    {

        std::array < vec3D, 8 > unitCube, rotCube, worldCube, projCube;
        auto &cell = mWorld.GetCell(vCell);

        unitCube[0] = { 0.0f, -fScale * Height, 0.0f };
        unitCube[1] = { fScale, -fScale * Height, 0.0f };
        unitCube[2] = { fScale, -fScale -fScale * Height, 0.0f };
        unitCube[3] = { 0.0f, -fScale -fScale* Height, 0.0f };
        unitCube[4] = { 0.0f, -fScale * Height, fScale };
        unitCube[5] = { fScale, -fScale * Height, fScale };
        unitCube[6] = { fScale, -fScale -fScale * Height, fScale };
        unitCube[7] = { 0.0f, -fScale -fScale * Height, fScale };

        // Translate Cube in x-z Plane  //

        for ( int i = 0; i < 8; i++ ) 
        {

            unitCube[i].x += ( vCell.x * fScale - Camera.x );
            unitCube[i].y +=  -Camera.y;
            unitCube[i].z += ( vCell.y * fScale - Camera.z );

        }

        // Rotate Cube in Y-Axis around Origin  //

        for ( int i = 0; i < 8; i++ ) 
        {

            rotCube[i].x = unitCube[i].x * cosf( fAngle ) + unitCube[i].z * sinf( fAngle ) ;
            rotCube[i].y = unitCube[i].y;
            rotCube[i].z = unitCube[i].x * -sinf( fAngle ) + unitCube[i].z * cosf( fAngle );

        }

        // Rotate Cube in X-Axis around Origin  //

        for ( int i = 0; i < 8; i++ ) 
        {

            worldCube[i].x = rotCube[i].x;
            worldCube[i].y = rotCube[i].y * cosf( Pitch ) - rotCube[i].z * sinf( Pitch ) ;
            worldCube[i].z = rotCube[i].y * sinf( Pitch ) + rotCube[i].z * cosf( Pitch );

        }

        // Ortographic Projection  //

        for ( int i = 0; i < 8; i++ ) 
        {

            projCube[i].x = worldCube[i].x + ScreenWidth() * 0.5f;
            projCube[i].y = worldCube[i].y + ScreenHeight() * 0.5f;
            projCube[i].z = worldCube[i].z;

        }

        return projCube;

    }

	bool OnUserCreate() override
	{

        RenderAllWall.Load( "./World.png" );
        Select.Load( "/home/drpanoukl4/PROGRAMACION/c++/OWWD/src/Demo/Selected.png" );

        mWorld.Create( 64, 64, 20 );

        for( int y = 0; y < mWorld.size.y; y ++ ) 
        {

            for( int x = 0; x < mWorld.size.x; x ++ )
            {

                for( int z = 0; z < mWorld.size.z; z ++ ) 
                {

                    //-- We Can Access Properties Because Function Return A Refrence & --//
                    mWorld.GetCell( {x, y, z} ).bWall = false;
                    mWorld.GetCell( {x, y, z} ).cHeight = 20;

                    mWorld.GetCell( {x, y, z} ).id[ Face::Floor ] = olc::vi2d{ 0, 0 } * TileSize;
                    mWorld.GetCell( {x, y, z} ).id[ Face::Top ] = olc::vi2d{ 1, 0 } * TileSize;
                    mWorld.GetCell( {x, y, z} ).id[ Face::North ] = olc::vi2d{ 0, 6 } * TileSize;
                    mWorld.GetCell( {x, y, z} ).id[ Face::South ] = olc::vi2d{ 0, 6 } * TileSize;
                    mWorld.GetCell( {x, y, z} ).id[ Face::West ] = olc::vi2d{ 0, 6 } * TileSize;
                    mWorld.GetCell( {x, y, z} ).id[ Face::East ] = olc::vi2d{ 0, 6 } * TileSize;

                }

            }


        }  

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

        //EditMode
        if( GetKey(olc::Key::T).bPressed )  
        {

            Wireframe = !Wireframe;

        }

        if( GetKey(olc::Key::B).bHeld ) 
        {

            olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

            if( GetKey(olc::Key::LEFT).bHeld ) {sCursor.x --; vTileCursor.x --;}
            if( GetKey(olc::Key::RIGHT).bHeld ) {sCursor.x ++; vTileCursor.x ++;} 
            if( GetKey(olc::Key::UP).bHeld ) {sCursor.y --; vTileCursor.y --;}
            if( GetKey(olc::Key::DOWN).bHeld ) {sCursor.y ++; vTileCursor.y ++;}

            if( sCursor.x < 0 ) sCursor.x = 0;
            if( sCursor.y < 0 ) sCursor.y = 0;

            if( sCursor.x >= 11) sCursor.x = 11 -1;
            if( sCursor.y >= 10) sCursor.y = 10 -1;

            if( vTileCursor.x < 0 ) vTileCursor.x = 0;
            if( vTileCursor.y < 0 ) vTileCursor.y = 0;

            if( vTileCursor.x >= 24) vTileCursor.x = 24 -1;
            if( vTileCursor.y >= 21) vTileCursor.y = 21 -1;

            eCamera = { sCursor.x,  sCursor.y};
            eCamera *= TileSize;

            DrawSprite( {0 - eCamera.x , 0 - eCamera.y}, RenderAllWall.sprite );
            DrawRect(vTileCursor * TileSize - eCamera, TileSize, olc::WHITE );

            if( GetMouse(0).bPressed ) 
            {
                vTileCursor = vMouse / TileSize + sCursor;
            }
            return true;

        }

        if( GetKey(olc::Key::W).bHeld ) fCameraPitch += 1.0f * fElapsedTime;
        if( GetKey(olc::Key::S).bHeld ) fCameraPitch -= 1.0f * fElapsedTime;

        if( GetKey(olc::Key::A).bHeld ) ftargetCameraAngle += 1.0f * fElapsedTime;
        if( GetKey(olc::Key::D).bHeld ) ftargetCameraAngle -= 1.0f * fElapsedTime;

        if( GetKey(olc::Key::Q).bHeld ) fCameraZoom += 5.0f * fElapsedTime;
        if( GetKey(olc::Key::Z).bHeld ) fCameraZoom -= 5.0f * fElapsedTime;

        fCameraAngle += ( ftargetCameraAngle - fCameraAngle ) * fElapsedTime;
        //fCameraPitch += ( ftargetCameraPitch - fCameraPitch ) * fElapsedTime; //xDD

        if( GetKey(olc::Key::LEFT).bHeld ) vCursor.x --;
        if( GetKey(olc::Key::RIGHT).bHeld ) vCursor.x ++;  
        if( GetKey(olc::Key::UP).bHeld ) vCursor.y --;
        if( GetKey(olc::Key::DOWN).bHeld ) vCursor.y ++;

        if( GetKey(olc::Key::P).bHeld ) vCursor.z ++;
        if( GetKey(olc::Key::M).bHeld ) vCursor.z --;

        if( vCursor.x < 0 ) vCursor.x = 0;
        if( vCursor.y < 0 ) vCursor.y = 0;
        if( vCursor.z < 0 ) vCursor.z = 0;

        if( vCursor.x >= mWorld.size.x ) vCursor.x = mWorld.size.x -1;
        if( vCursor.y >= mWorld.size.y ) vCursor.y = mWorld.size.y - 1;
        if( vCursor.z >= mWorld.size.z ) vCursor.z = mWorld.size.z - 1;

        Camera = { vCursor.x + 0.5f,  vCursor.y + 0.5f };
        Camera *= fCameraZoom;

        if( GetKey(olc::Key::SPACE).bPressed )
        {   

            if( vCursor.z >= 1 ) 
            {

                mWorld.GetCell( vCursor ).bExist = !mWorld.GetCell( vCursor ).bExist;

            }

            mWorld.GetCell( vCursor ).bWall = !mWorld.GetCell( vCursor ).bWall;

        }

        if( GetKey(olc::Key::F).bPressed ) mWorld.GetCell(vCursor).id[ Face::Floor ] = vTileCursor * TileSize;
        if( GetKey(olc::Key::G).bPressed ) mWorld.GetCell(vCursor).id[ Face::East ] = vTileCursor * TileSize;
        if( GetKey(olc::Key::H).bPressed ) mWorld.GetCell(vCursor).id[ Face::South ] = vTileCursor * TileSize;
        if( GetKey(olc::Key::J).bPressed ) mWorld.GetCell(vCursor).id[ Face::West ] = vTileCursor * TileSize;
        if( GetKey(olc::Key::K).bPressed ) mWorld.GetCell(vCursor).id[ Face::North ] = vTileCursor * TileSize;
        if( GetKey(olc::Key::L).bPressed ) mWorld.GetCell(vCursor).id[ Face::Top ] = vTileCursor * TileSize; 

        //Choose wich cube going to Draw

        std::array < vec3D, 8 > cullCube = CreateCube( {0, 0}, fCameraAngle, fCameraPitch, fCameraZoom, { Camera.x, 0.0f, Camera.y }, 0 );
        CalculateVisibleFace( cullCube );

        std::vector < sQuad > sQuads;

		//Draawing
        for( int y = 0; y < mWorld.size.y; y ++ ) 
        {

            for( int x = 0; x < mWorld.size.x; x ++ )
            {

                int cHeights = mWorld.GetCell( {x, y, 0} ).cHeight;
                getFaceQuad({ x, y } , fCameraAngle, fCameraPitch, fCameraZoom, { Camera.x, 0.0f, Camera.y }, sQuads, 0 );

                for( int h = 0; h < cHeights; h ++ ) 
                {
                    
                    if( mWorld.GetCell( {x, y, h} ).bExist == true ) 
                    {

                        getFaceQuad({ x, y, h } , fCameraAngle, fCameraPitch, fCameraZoom, { Camera.x, 0.0f, Camera.y }, sQuads, h );

                    }

        
                }

            }

        }

        //  **- Draw In Correct order -** From Farthest away to Closest//

        std::sort( sQuads.begin(), sQuads.end(), []( const sQuad &q1, const sQuad &q2 ) 
        {

            float z1 = ( q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z ) * 0.25f;
            float z2 = ( q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z ) * 0.25f;

            return z1 < z2;

        } );

        Clear(olc::BLACK);

        //-DRAWING-//

        if( !Wireframe ) 
        {

            for( auto & q : sQuads ) 
            {

                DrawPartialWarpedDecal
                (

                    RenderAllWall.decal,
                    { { q.points[0].x, q.points[0].y }, { q.points[1].x, q.points[1].y }, { q.points[2].x, q.points[2].y }, { q.points[3].x, q.points[3].y } },
                    q.tile,
                    TileSize

                );

            }

            sQuads.clear();

            getFaceQuad(vCursor , fCameraAngle, fCameraPitch, fCameraZoom, { Camera.x, 0.0f, Camera.y }, sQuads, vCursor.z );
        
            for( auto & q : sQuads ) 
                DrawWarpedDecal
                (

                    Select.decal,
                    { { q.points[0].x, q.points[0].y }, { q.points[1].x, q.points[1].y }, { q.points[2].x, q.points[2].y }, { q.points[3].x, q.points[3].y } }

                );

        }else 
        {

            for( auto & q : sQuads ) 
            {

                DrawLine( q.points[0].x, q.points[0].y, q.points[1].x, q.points[1].y, olc::WHITE );
                DrawLine( q.points[1].x, q.points[1].y, q.points[2].x, q.points[2].y, olc::WHITE );
                DrawLine( q.points[2].x, q.points[2].y, q.points[3].x, q.points[3].y, olc::WHITE );
                DrawLine( q.points[3].x, q.points[3].y, q.points[0].x, q.points[0].y, olc::WHITE );

                for( auto &r : q.points )
                {

                    FillCircle( r.x, r.y, 1.0f, olc::Pixel(129, 228, 220) );

                }
    
            }


        }

		return true;
	}
};

int main()
{
	Shadow demo;
	if (demo.Construct(400, 400, 4, 4))
		demo.Start();

	return 0;
}