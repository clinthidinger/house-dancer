#include <cinder/app/App.h>
#include <cinder/gl/Texture.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/Utilities.h>
#include <cinder/CinderImGui.h>
#include <cinder/Log.h>
#include <cinder/Timeline.h>
#include <cinder/Tween.h>
#include <imgui/imgui_internal.h>
#include <Kinect2.h>
#include "LinkWrapper.h"

#include "fonts/RobotoRegular.h"
#include "SavitzkyGolayFilter.h"

struct AnimatedRing
{
	ci::Anim<float> life{ 1.0f };
	ci::vec3 pos{ 0.0f };
	double beatFract{ 0.0 };
	AnimatedRing( float bpm, const ci::vec3 &pos_, double beatFract_ )
		: pos( pos_ )
		, beatFract( beatFract_ )
	{
		const float startLife = ( 60.0f / bpm ) * 2;
		CI_LOG_I( "StartLife " + std::to_string( startLife ) );
		ci::app::timeline().apply( &life, 1.0f, 0.0f, startLife, ci::EaseOutQuad() );
	}
};

class HouseDancerApp : public ci::app::App
{
public:
	void draw() override;
	void setup() override;
	void update() override;

private:
	struct Foot
	{
		bool isUp{ false };
		bool isDown{ true };
		bool hasEmittedRing{ false };
	};
	struct Knee
	{
		bool isUp{ false };
		bool hasEmittedRing{ false };
		float yPrevPos{ 0.0f };
		float yPrevVel{ 0.0f };
	};
	struct BodyTrackState
	{
		Foot lFoot;
		Foot rFoot;
		Knee lKnee;
		Knee rKnee;
		float standingKneeY{ 0.0f };
		float standingHipY{ 0.0f };
		bool isKneeCalibrated{ false };
	};

	void updateImGui();
	void track( const Kinect2::Body &body );
	void detectFootStep( 
		const ci::vec3 &footPos,
		const ci::vec3 &kneePos,
		BodyTrackState &trackState,
		Foot &foot,
		Knee &knee,
		float hipY 
	);
	void detectKneeRaise( BodyTrackState &trackState, const ci::vec3 &kneePos, Knee &knee );
	void cleanupInactiveRings();
	void setupCamera();
	void drawRing( const ci::vec3 &pos, float scale, const ci::ColorAf &color );
	static double fract( double );
	static ci::vec3 kinectToCinder( const ci::vec3 &pos );
	static ci::Colorf getRingColor( double fract );
	bool hasTrackedBody() const;

	Kinect2::BodyFrame mBodyFrame;
	ci::Channel8uRef mChannelBodyIndex;
	ci::Channel16uRef mChannelDepth;
	Kinect2::DeviceRef mDevice;
	LinkWrapper mLinkWrapper;

	float mFrameRate;
	bool mFullScreen;
	
	ImFont *mFont{ nullptr };
	//ImFont *mFontAwesomeTweaked{ nullptr };
	static constexpr const int DefaultFontSize{ 20 };
	int mFontSize{ DefaultFontSize };
	
	std::vector<std::unique_ptr<AnimatedRing>> mFootRings;
	std::vector<std::unique_ptr<AnimatedRing>> mKneeRings;

	std::map<uint64_t, ci::vec3> mPrevLeftFoot;
	std::map<uint64_t, ci::vec3> mPrevRightFoot;
	std::map<uint64_t, ci::vec3> mPrevLeftKnee;
	std::map<uint64_t, ci::vec3> mPrevRightKnee;

	std::unordered_map<uint64_t, BodyTrackState> mTrackStates;

	float mStepThreshold{ 0.15f };
	float mKneeRaiseThreshold{ 0.2f };

	ci::gl::VertBatchRef mGridBatch;
	ci::CameraPersp mCam;

	float mFloorY{ 1000.0f };
	bool mHasFloorY{ false };
	ci::gl::BatchRef mRingBatch;
	bool mHasTrackedBodies{ false };
	static constexpr float FootUpThresh = 0.02f;
	static constexpr float FootDownThresh = 0.01f;
};

inline ci::vec3 HouseDancerApp::kinectToCinder( const ci::vec3 &pos )
{
	return ci::vec3( -pos.x, pos.y, pos.z );
}

inline double HouseDancerApp::fract( double f)
{
	return f - static_cast<long>( f );
}

inline bool HouseDancerApp::hasTrackedBody() const
{
	if( mBodyFrame.getBodies().empty() )
	{
		return false;
	}
	for( auto const &body : mBodyFrame.getBodies() )
	{
		if( body.isTracked() )
		{
			return true;
		}
	}

	return false;
}

ci::Colorf HouseDancerApp::getRingColor( double fract )
{
	constexpr double thresh = 0.15;
	if( ( fract < thresh ) || ( fract > ( 1.0 - thresh ) ) ) // Beat (down)
	{
		return ci::Colorf( 0.1f, 0.4f, 1.0f );
	}
	if( ( fract > ( 0.5 - thresh ) ) && ( fract < ( 0.5 + thresh ) ) ) // And (up)
	{
		return ci::Colorf( 0.1f, 1.0f, 0.4f );
	}
	//return ci::Colorf( 1.0f, 0.1f, 0.0f );
	return ci::Colorf( 0.0f, 0.0f, 0.0f );
}

void HouseDancerApp::draw()
{
	ci::gl::viewport( getWindowSize() );
	ci::gl::clear( ci::Colorf::black() );
	ci::gl::color( ci::ColorAf::white() );
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	ci::gl::enableAlphaBlending();

	 if ( mChannelDepth ) 
     {
		 if( !hasTrackedBody() )
		 {
			 ci::gl::enable( GL_TEXTURE_2D );
			 ci::gl::TextureRef tex = ci::gl::Texture::create( *Kinect2::channel16To8( mChannelDepth ) );
			 ci::gl::draw( tex, tex->getBounds(), ci::Rectf( getWindowBounds() ) );
		 }
	 }

	if ( mChannelBodyIndex ) 
    {
		ci::gl::enable( GL_TEXTURE_2D );
		ci::gl::color( ci::ColorAf( ci::Colorf::white(), 0.15f ) );
		ci::gl::TextureRef tex = ci::gl::Texture::create( *Kinect2::colorizeBodyIndex( mChannelBodyIndex ) );
		ci::gl::draw( tex, tex->getBounds(), ci::Rectf( getWindowBounds() ) );

		
		ci::gl::ScopedModelMatrix scopedMdlMtx;
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
			}
		}
		//ci::gl::popMatrices();
	}

	{
		ci::gl::ScopedMatrices scopedMatrices;
		ci::gl::setMatrices( mCam );
		ci::gl::ScopedDepth scopeDepth( true );

		{
			ci::gl::ScopedLineWidth scopedLineWidth( 2.0f );
			ci::gl::ScopedColor scopedColor( ci::Colorf::white() );
			ci::gl::ScopedModelMatrix scopedModel;
			ci::gl::translate( 0.0f, mFloorY * 1.0, 0.0f );
			mGridBatch->draw();
		}

		for( const Kinect2::Body &body : mBodyFrame.getBodies() )
		{
			auto const &jointMap = body.getJointMap();
			if( body.isTracked() && !jointMap.empty() )
			{
				ci::vec3 leftFoot = jointMap.at( JointType_FootLeft ).getPosition();
				ci::vec3 rightFoot = jointMap.at( JointType_FootRight ).getPosition();
				leftFoot.x = -leftFoot.x;
				rightFoot.x = -rightFoot.x;

				//if( !mHasFloorY )
				{
					if( std::abs( leftFoot.y - rightFoot.y ) < 0.001 )
					{
						float floorY = std::min( leftFoot.y, rightFoot.y );
						// or if floory is less than current floory
						mFloorY = std::min( floorY, mFloorY );
						mHasFloorY = true;
					}
				}
			}// end if
		}// end for

		constexpr float startRingScale = 0.12f;
		constexpr float endRingScale = 0.18f;
		ci::gl::ScopedBlend blend( GL_SRC_ALPHA, GL_ONE );
		for( const auto &ring : mFootRings )
		{
			const float alpha = ring->life.value();
			const float scale = ci::lerp<float>( 1.0f - alpha, endRingScale, startRingScale );
			drawRing( ring->pos, scale, ci::ColorAf( getRingColor( ring->beatFract ), alpha));
		}
		for( const auto &ring : mKneeRings )
		{
			const float alpha = ring->life.value();
			const float scale = ci::lerp<float>( 1.0f - alpha, endRingScale, startRingScale );
			drawRing( ring->pos, scale, ci::ColorAf( getRingColor( ring->beatFract ), alpha ) );
		}
	}
}

void HouseDancerApp::setup()
{
	mFrameRate	= 0.0f;
	mFullScreen	= false;

	mDevice = Kinect2::Device::create();
	mDevice->start();
	mDevice->connectBodyEventHandler( [this]( const Kinect2::BodyFrame frame )
	{
		mBodyFrame = frame;
	} );
	mDevice->connectBodyIndexEventHandler( [this]( const Kinect2::BodyIndexFrame frame )
	{
		mChannelBodyIndex = frame.getChannel();
	} );
	 mDevice->connectDepthEventHandler( [this]( const Kinect2::DepthFrame frame )
	 {
		if( !hasTrackedBody() )
		{
	 		mChannelDepth = frame.getChannel();
		}
	 } );
	
	ImGui::Initialize();
	ImFontConfig fontConfig;
	fontConfig.FontDataOwnedByAtlas = false;
	mFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF( const_cast< unsigned char * >( RobotoRegular ), RobotoRegularLength, mFontSize, &fontConfig );

	mGridBatch = ci::gl::VertBatch::create( GL_LINES );
	const float halfSize = 3.0f; //m
	const float gridStep = 0.20f;
	for( float xzVal = -halfSize; xzVal <= halfSize; xzVal += gridStep )
	{
		mGridBatch->vertex( xzVal, 0.0f, -halfSize );
		mGridBatch->vertex( xzVal, 0.0f,  halfSize );

		mGridBatch->vertex( -halfSize, 0.0f, xzVal );
		mGridBatch->vertex(  halfSize, 0.0f, xzVal );
	}

	setupCamera();

	auto ring = ci::geom::Ring()
		.radius( 1.0f )
		.width( 0.2f )
		.subdivisions( 64 )
		;
	mRingBatch = ci::gl::Batch::create( ring, ci::gl::getStockShader( ci::gl::ShaderDef().color() ) );
}

void HouseDancerApp::drawRing( const ci::vec3 &pos, float scale, const ci::ColorAf &color )
{
	ci::gl::ScopedColor colorScope( color );
	ci::gl::ScopedModelMatrix scopedModelMtx;

	ci::gl::translate( pos );
	ci::gl::rotate( ci::toRadians( 90.0f ), 1, 0, 0 );
	ci::gl::scale( scale, scale, scale );

	mRingBatch->draw();
}

void HouseDancerApp::update()
{
	mFrameRate = getAverageFps();
	
	if ( mFullScreen != isFullScreen() ) 
    {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

	if( mDevice )
	{
		for( const auto &body : mBodyFrame.getBodies() ) 
		{
			if( body.isTracked() )
			{
				track( body );
			}
		}
		cleanupInactiveRings();
	}

	updateImGui();
}

void HouseDancerApp::updateImGui()
{
	ImGui::SetCurrentFont( mFont );

	ImGui::Begin( "Controls" );
	ImGui::Text( "Frame Rate: %.2f", mFrameRate );
	ImGui::Checkbox( "Is FullScreen", &mFullScreen );
	ImGui::Text( "Peers: %d", mLinkWrapper.getNumPeers() );
	if( ImGui::Button( "Connect" ) )
	{
		mLinkWrapper.setIsEnabled( true );
	}

	double phase = 0.0;
	const float beat = mLinkWrapper.getBeatAndPhase( phase );
	const double tempo = mLinkWrapper.getTempo();
	ImGui::Text( "Tempo: %.2f", tempo );
	ImGui::Text( "Beat: %.2f", beat );
	ImGui::Text( "Phase: %.2f", phase );

	ImGui::End();

	auto *drawList = ImGui::GetBackgroundDrawList();
	const std::string text = std::to_string( static_cast<int>( mLinkWrapper.getTempo() ) ) + " : " + 
		                     std::to_string( static_cast<int>( mLinkWrapper.getPhase() ) + 1 );
	drawList->AddText( mFont, 80, ImVec2( getWindowWidth() - ImGui::GetFontSize() * 10,  0 ), IM_COL32_WHITE, text.c_str() );
}

void HouseDancerApp::detectFootStep( 
	const ci::vec3 &footPos,
	const ci::vec3 &kneePos,
	BodyTrackState &trackState,
	Foot &foot,
	Knee &knee,
	float hipY
)
{
	if( !foot.isUp )
	{
		if( footPos.y > ( mFloorY + FootUpThresh ) )
		{
			foot.isUp = true;
			foot.isDown = false;
			foot.hasEmittedRing = false;
			CI_LOG_I( "Foot up" );
		}
	}
	if( !foot.isDown )
	{
		if( footPos.y < ( mFloorY + FootDownThresh ) )
		{
			if( foot.isUp )
			{
				mFootRings.push_back( std::make_unique<AnimatedRing>( mLinkWrapper.getTempo(), footPos, fract( mLinkWrapper.getBeat() ) ) );
				foot.hasEmittedRing = true;
				CI_LOG_I( "Foot emit " + std::to_string( ci::app::getElapsedFrames() ) );
			}
			foot.isUp = false;
			foot.isDown = true;
			CI_LOG_I( "Foot down" );
			//if( trackState.mStandingKneeY < ci::EPSILON_VALUE )
			{
				if( hipY > trackState.standingHipY )
				{
					trackState.standingHipY = hipY;
					trackState.standingKneeY = kneePos.y;
				}
				trackState.isKneeCalibrated = true;
			}
		}
	}
}

void HouseDancerApp::detectKneeRaise( BodyTrackState &trackState, const ci::vec3 &kneePos, Knee &knee )
{
	if( !trackState.isKneeCalibrated )
	{
		return;
	}
	// assymetry???
	const float kneeUpThresh = ci::lerp( trackState.standingKneeY, trackState.standingHipY, 0.20f );
	const float kneeDownThresh = ci::lerp( trackState.standingKneeY, trackState.standingHipY, 0.10f );

	if( knee.isUp )
	{
		const float vel = kneePos.y - knee.yPrevPos;
		if( !knee.hasEmittedRing && vel < 0.0f && knee.yPrevVel < 0.0f )
		{
			// emit ring;
			mKneeRings.push_back( std::make_unique<AnimatedRing>( mLinkWrapper.getTempo(), kneePos, fract( mLinkWrapper.getBeat() ) ) );
			knee.hasEmittedRing = true;
			CI_LOG_I( "Knee emit" );
		}
		knee.yPrevVel = vel;
		if( kneePos.y < kneeDownThresh )
		{
			knee.isUp = false;
			knee.hasEmittedRing = false;
			knee.yPrevVel = 0.0f;
			knee.yPrevPos = 0.0f;
			CI_LOG_I( "Knee down" );
		}
	}
	else
	{
		if( kneePos.y > kneeUpThresh )
		{
			knee.isUp = true;
			knee.yPrevVel = 0.0f;
			knee.yPrevPos = kneePos.y;
			CI_LOG_I( "Knee up" );
		}
	}
}

void HouseDancerApp::track( const Kinect2::Body &body )
{
	uint64_t trackingId = body.getId();

	// Get foot positions
	auto const &jointMap = body.getJointMap();
	const ci::vec3 &leftFootPos = kinectToCinder( jointMap.at( JointType_FootLeft ).getPosition() );
	const ci::vec3 &rightFootPos = kinectToCinder( jointMap.at( JointType_FootRight ).getPosition() );
	const ci::vec3 &leftKneePos = kinectToCinder( jointMap.at( JointType_KneeLeft ).getPosition() );
	const ci::vec3 &rightKneePos = kinectToCinder( jointMap.at( JointType_KneeRight ).getPosition() );
	const ci::vec3 &leftHipPos = kinectToCinder( jointMap.at( JointType_HipLeft ).getPosition() );
	const ci::vec3 &rightHipPos = kinectToCinder( jointMap.at( JointType_HipRight ).getPosition() );

	//CI_LOG_I( "Left foot: "  + std::to_string(leftFoot.x) + " " + std::to_string( leftFoot.y ) + " " + std::to_string( leftFoot.z ) );
	auto iter = mTrackStates.find( body.getId() );
	if( iter == mTrackStates.end() )
	{
		iter = mTrackStates.emplace( body.getId(), BodyTrackState() ).first;
	}
	auto &trackState = iter->second;

	detectFootStep( leftFootPos, leftKneePos, trackState, trackState.lFoot, trackState.lKnee, leftHipPos.y );
	detectFootStep( rightFootPos, rightKneePos, trackState, trackState.rFoot, trackState.rKnee, rightHipPos.y );
	detectKneeRaise( trackState, leftKneePos, trackState.lKnee );
	detectKneeRaise( trackState, rightKneePos, trackState.rKnee );
}

void HouseDancerApp::cleanupInactiveRings()
{
	mFootRings.erase(
		std::remove_if( mFootRings.begin(), mFootRings.end(),
			[] ( const std::unique_ptr<AnimatedRing> &ring ) { return ring->life < ci::EPSILON_VALUE; } ),
		mFootRings.end() );
	mKneeRings.erase(
		std::remove_if( mKneeRings.begin(), mKneeRings.end(),
			[] ( const std::unique_ptr<AnimatedRing> &ring ) { return ring->life < ci::EPSILON_VALUE; } ),
		mKneeRings.end() );
}

void HouseDancerApp::setupCamera()
{
	constexpr float depthWidth = 512.0f;
	constexpr float depthHeight = 424.0f;
	constexpr float fx = 366.1f;
	constexpr float fy = 366.1f;

	const float fovY = 2.0f * atanf( ( depthHeight / 2.0f ) / fy );
	const float fovDegrees = glm::degrees( fovY );
	const float aspect = depthWidth / depthHeight;

	mCam.setPerspective( fovDegrees, aspect, 0.1f, 8.0f );
	mCam.lookAt( ci::vec3( 0, 0, 0 ), ci::vec3( 0, 0, 1 ) ); // Match Kinect view
}

CINDER_APP( HouseDancerApp, ci::app::RendererGl, []( ci::app::App::Settings* settings )
{
	settings->prepareWindow( ci::app::Window::Format().size( 1024, 768 ).title( "House Dancer App" ) );
	settings->setFrameRate( 60.0f );
} )
