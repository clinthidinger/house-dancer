#include <cinder/app/App.h>
#include <cinder/gl/Texture.h>
#include <cinder/params/Params.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/Utilities.h>
#include <cinder/CinderImGui.h>
#include <cinder/Log.h>
#include <imgui/imgui_internal.h>
#include <Kinect2.h>

class HouseDancerApp : public ci::app::App
{
public:
	void draw() override;
	void setup() override;
	void update() override;
private:
	Kinect2::BodyFrame			mBodyFrame;
	ci::Channel8uRef			mChannelBodyIndex;
	//ci::Channel16uRef			mChannelDepth;
	Kinect2::DeviceRef			mDevice;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

void HouseDancerApp::draw()
{
	ci::gl::viewport( getWindowSize() );
	ci::gl::clear( ci::Colorf::black() );
	ci::gl::color( ci::ColorAf::white() );
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	ci::gl::enableAlphaBlending();

	// if ( mChannelDepth ) 
    // {
	// 	ci::gl::enable( GL_TEXTURE_2D );
	// 	ci::gl::TextureRef tex = gl::Texture::create( *Kinect2::channel16To8( mChannelDepth ) );
	// 	ci::gl::draw( tex, tex->getBounds(), Rectf( getWindowBounds() ) );
	// }

	if ( mChannelBodyIndex ) 
    {
		ci::gl::enable( GL_TEXTURE_2D );
		ci::gl::color( ci::ColorAf( ci::Colorf::white(), 0.15f ) );
		ci::gl::TextureRef tex = ci::gl::Texture::create( *Kinect2::colorizeBodyIndex( mChannelBodyIndex ) );
		ci::gl::draw( tex, tex->getBounds(), ci::Rectf( getWindowBounds() ) );

		auto drawHand = [ & ]( const Kinect2::Body::Hand& hand, const ci::ivec2& pos ) -> void
		{
			switch ( hand.getState() ) 
            {
			case HandState_Closed:
				ci::gl::color( ci::ColorAf( 1.0f, 0.0f, 0.0f, 0.5f ) );
				break;
			case HandState_Lasso:
				ci::gl::color( ci::ColorAf( 0.0f, 0.0f, 1.0f, 0.5f ) );
				break;
			case HandState_Open:
				ci::gl::color( ci::ColorAf( 0.0f, 1.0f, 0.0f, 0.5f ) );
				break;
			default:
				ci::gl::color( ci::ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
				break;
			}
			ci::gl::drawSolidCircle( pos, 30.0f, 32 );
		};

		ci::gl::pushMatrices();
		ci::gl::scale( ci::vec2( getWindowSize() ) / ci::vec2( mChannelBodyIndex->getSize() ) );
		ci::gl::disable( GL_TEXTURE_2D );
		for ( const Kinect2::Body& body : mBodyFrame.getBodies() ) 
        {
			if ( body.isTracked() ) 
            {
				ci::gl::color( ci::ColorAf::white() );
				for ( const auto& joint : body.getJointMap() ) 
                {
					if ( joint.second.getTrackingState() == TrackingState::TrackingState_Tracked ) 
                    {
						ci::vec2 pos( mDevice->mapCameraToDepth( joint.second.getPosition() ) );
						ci::gl::drawSolidCircle( pos, 5.0f, 32 );
						ci::vec2 parent( mDevice->mapCameraToDepth(
							body.getJointMap().at( joint.second.getParentJoint() ).getPosition()
							) );
						ci::gl::drawLine( pos, parent );
					}
				}
				drawHand( body.getHandLeft(), mDevice->mapCameraToDepth( body.getJointMap().at( JointType_HandLeft ).getPosition() ) );
				drawHand( body.getHandRight(), mDevice->mapCameraToDepth( body.getJointMap().at( JointType_HandRight ).getPosition() ) );
			}
		}
		ci::gl::popMatrices();
	}

	mParams->draw();
}

void HouseDancerApp::setup()
{
	mFrameRate	= 0.0f;
	mFullScreen	= false;

	mDevice = Kinect2::Device::create();
	mDevice->start();
	mDevice->connectBodyEventHandler( [ & ]( const Kinect2::BodyFrame frame )
	{
		mBodyFrame = frame;
	} );
	mDevice->connectBodyIndexEventHandler( [ & ]( const Kinect2::BodyIndexFrame frame )
	{
		mChannelBodyIndex = frame.getChannel();
	} );
	// mDevice->connectDepthEventHandler( [ & ]( const Kinect2::DepthFrame frame )
	// {
	// 	mChannelDepth = frame.getChannel();
	// } );
	
	ImGui::Initialize();

	mParams = ci::params::InterfaceGl::create( "Params", ci::ivec2( 200, 100 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,			"", true );
	mParams->addParam( "Full screen",	&mFullScreen ).key( "f" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },	"key=q" );
}

void HouseDancerApp::update()
{
	mFrameRate = getAverageFps();
	
	if ( mFullScreen != isFullScreen() ) 
    {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
}

CINDER_APP( HouseDancerApp, ci::app::RendererGl, []( ci::app::App::Settings* settings )
{
	settings->prepareWindow( ci::app::Window::Format().size( 1024, 768 ).title( "House Dancer App" ) );
	settings->setFrameRate( 60.0f );
} )
