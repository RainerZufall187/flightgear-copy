/* Implementation of 'step' view system.

The basic idea here is calculate view position and direction by a series
of explicit steps. Steps can move to the origin of the user aircraft or a
multiplayer aircraft, or modify the current position by a fixed vector (e.g.
to move from aircraft origin to the pilot's eyepoint), or rotate the current
direction by a fixed transformation etc. We can also have a step that sets the
direction to point to a previously-calculated position.

This is similar to what is already done by the View code, but making the
individual steps explicit gives us more flexibility.

We also allow views to be defined and created at runtime instead of being
hard-coded in *-set.xml files. For example this would make it possible to
define a view from the user's aircraft's pilot to the centre of a multiplayer
aircraft (or to a multiplayer aircraft's pilot).

The dynamic nature of step views allows view cloning with composite-viewer.

*/     

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "sview.hxx"

#include <simgear/debug/logstream.hxx>
#include <simgear/math/SGMath.hxx>
#include <simgear/props/props.hxx>

#include <Main/globals.hxx>

#include <simgear/scene/util/OsgMath.hxx>

#include <osgViewer/CompositeViewer>


/* Position and direction. */
struct SviewPosDir
{
    SGGeod  position;
    double  heading;
    double  pitch;
    double  roll;
    
    /* Only used by SviewStepTarget; usually will be from a previously
    evaluated Sview. */
    SGGeod  target;
    
    /* The final position and direction, in a form suitable for setting an
    osg::Camera's view matrix. */
    SGVec3d position2;
    SGQuatd direction2;
};


/* Abstract base class for a single view step. A view step modifies a
SviewPosDir, e.g. translating the position and/or rotating the direction. */
struct SviewStep
{
    /* Updates <posdir>. */
    virtual void evaluate(SviewPosDir& posdir) = 0;
    
    virtual ~SviewStep() {}
};


/* Sets position to aircraft origin and direction to aircraft's longitudal
axis. */
struct SviewStepAircraft : SviewStep
{
    /* For the user aircraft, <root> should be /. For a multiplayer
    aircraft, it should be /ai/models/multiplayer[]. */
    SviewStepAircraft(SGPropertyNode* root)
    {
        m_longitude   = root->getNode("position/longitude-deg");
        m_latitude    = root->getNode("position/latitude-deg");
        m_altitude    = root->getNode("position/altitude-ft");
        m_heading     = root->getNode("orientation/true-heading-deg");
        m_pitch       = root->getNode("orientation/pitch-deg");
        m_roll        = root->getNode("orientation/roll-deg");
    }
    
    void evaluate(SviewPosDir& posdir) override
    {
        posdir.position = SGGeod::fromDegFt(
                m_longitude->getDoubleValue(),
                m_latitude->getDoubleValue(),
                m_altitude->getDoubleValue()
                );

        posdir.heading  = m_heading->getDoubleValue();
        posdir.pitch    = m_pitch->getDoubleValue();
        posdir.roll     = m_roll->getDoubleValue();
    }
    
    private:
    
    SGPropertyNode_ptr  m_longitude;
    SGPropertyNode_ptr  m_latitude;
    SGPropertyNode_ptr  m_altitude;
    
    SGPropertyNode_ptr  m_heading;
    SGPropertyNode_ptr  m_pitch;
    SGPropertyNode_ptr  m_roll;
};


/* Moves position by fixed vector, does not change direction.

E.g. for moving from aircraft origin to pilot viewpoint. */
struct SviewStepMove : SviewStep
{
    SviewStepMove(double forward, double up, double right)
    :
    m_offset(right, -up, -forward)
    {
        SG_LOG(SG_VIEW, SG_INFO, "forward=" << forward << " up=" << up << " right=" << right);
    }
    
    void evaluate(SviewPosDir& posdir) override
    {
        /* These calculations are copied from View::recalcLookFrom(). */
        
        // The rotation rotating from the earth centerd frame to
        // the horizontal local frame
        SGQuatd hlOr = SGQuatd::fromLonLat(posdir.position);
        
        // The rotation from the horizontal local frame to the basic view orientation
        SGQuatd hlToBody = SGQuatd::fromYawPitchRollDeg(posdir.heading, posdir.pitch, posdir.roll);
        
        // Compute the eyepoints orientation and position
        // wrt the earth centered frame - that is global coorinates
        SGQuatd ec2body = hlOr * hlToBody;
        
        // The cartesian position of the basic view coordinate
        SGVec3d position = SGVec3d::fromGeod(posdir.position);
        
        // This is rotates the x-forward, y-right, z-down coordinate system the where
        // simulation runs into the OpenGL camera system with x-right, y-up, z-back.
        SGQuatd q(-0.5, -0.5, 0.5, 0.5);
        
        position += (ec2body * q).backTransform(m_offset);
        posdir.position = SGGeod::fromCart(position);
    }
    
    private:
    SGVec3d m_offset;
};


/* Modifies heading, pitch and roll by fixed amounts; does not change position.

E.g. can be used to preserve direction (relative to aircraft) of Helicopter
view at the time it was cloned. */
struct SviewStepRotate : SviewStep
{
    SviewStepRotate(double heading=0, double pitch=0, double roll=0)
    :
    m_heading(heading),
    m_pitch(pitch),
    m_roll(roll)
    {
        SG_LOG(SG_VIEW, SG_INFO, "heading=" << heading << " pitch=" << pitch << " roll=" << roll);
    }
    
    void evaluate(SviewPosDir& posdir) override
    {
        /* Should we use SGQuatd to evaluate things? */
        posdir.heading += -m_heading;
        posdir.pitch += -m_pitch;
        posdir.roll += m_roll;
    }
    
    private:
    double  m_heading;
    double  m_pitch;
    double  m_roll;
};

/* Multiply heading, pitch and roll by constants. */
struct SviewStepDirectionMultiply : SviewStep
{
    SviewStepDirectionMultiply(double heading=0, double pitch=0, double roll=0)
    :
    m_heading(heading),
    m_pitch(pitch),
    m_roll(roll)
    {
        SG_LOG(SG_VIEW, SG_INFO, "heading=" << heading << " pitch=" << pitch << " roll=" << roll);
    }
    
    void evaluate(SviewPosDir& posdir) override
    {
        posdir.heading *= m_heading;
        posdir.pitch *= m_pitch;
        posdir.roll *= m_roll;
    }
    
    private:
    double  m_heading;
    double  m_pitch;
    double  m_roll;
};

/* Copies current position to posdir.target. Used by SviewEyeTarget() to make
current position be available as target later on, e.g. by SviewStepTarget. */
struct SviewStepCopyToTarget : SviewStep
{
    void evaluate(SviewPosDir& posdir) override
    {
        posdir.target = posdir.position;
    }
};


/* Move position to nearest tower. */
struct SviewStepNearestTower : SviewStep
{
    /* For user aircraft <sim> should be /sim; for multiplayer aircraft it
    should be ai/models/multiplayer[]/sim. */
    SviewStepNearestTower(SGPropertyNode* sim)
    :
    m_latitude(sim->getNode("tower/latitude-deg")),
    m_longitude(sim->getNode("tower/longitude-deg")),
    m_altitude(sim->getNode("tower/altitude-ft"))
    {
    }
    
    void evaluate(SviewPosDir& posdir) override
    {
        posdir.position = SGGeod::fromDegFt(
                m_longitude->getDoubleValue(),
                m_latitude->getDoubleValue(),
                m_altitude->getDoubleValue()
                );
        SG_LOG(SG_VIEW, SG_BULK, "moved posdir.postion to: " << posdir.position);
    }
    
    SGPropertyNode_ptr m_latitude;
    SGPropertyNode_ptr m_longitude;
    SGPropertyNode_ptr m_altitude;
};


/* Rotates view direction to point at a previously-calculated target. */
struct SviewStepTarget : SviewStep
{
    /* Alters posdir.direction to point to posdir.target. */
    void evaluate(SviewPosDir& posdir) override
    {
        /* See View::recalcLookAt(). */
        
        SGQuatd geodEyeOr = SGQuatd::fromYawPitchRollDeg(posdir.heading, posdir.pitch, posdir.roll);
        SGQuatd geodEyeHlOr = SGQuatd::fromLonLat(posdir.position);

        SGQuatd ec2eye = geodEyeHlOr*geodEyeOr;
        SGVec3d eyeCart = SGVec3d::fromGeod(posdir.position);

        SGVec3d atCart = SGVec3d::fromGeod(posdir.target);

        // add target offsets to at_position...
        // Compute the eyepoints orientation and position
        // wrt the earth centered frame - that is global coorinates
        //_absolute_view_pos = eyeCart;

        // the view direction
        SGVec3d dir = normalize(atCart - eyeCart);
        // the up directon
        SGVec3d up = ec2eye.backTransform(SGVec3d(0, 0, -1));
        // rotate -dir to the 2-th unit vector
        // rotate up to 1-th unit vector
        // Note that this matches the OpenGL camera coordinate system
        // with x-right, y-up, z-back.
        posdir.direction2 = SGQuatd::fromRotateTo(-dir, 2, up, 1);
        
        posdir.position2 = SGVec3d::fromGeod(posdir.position);
        
        SG_LOG(SG_VIEW, SG_BULK, "have set posdir.position2: " << posdir.position2);
        SG_LOG(SG_VIEW, SG_BULK, "have set posdir.direction2: " << posdir.direction2);
    }
};


/* Converts position, heading, pitch and roll into position2 and direction2,
which will be used to set the camera parameters. */
struct SviewStepFinal : SviewStep
{
    void evaluate(SviewPosDir& posdir) override
    {
        /* See View::recalcLookFrom(). */
        
        // The rotation rotating from the earth centerd frame to
        // the horizontal local frame
        SGQuatd hlOr = SGQuatd::fromLonLat(posdir.position);

        // The rotation from the horizontal local frame to the basic view orientation
        SGQuatd hlToBody = SGQuatd::fromYawPitchRollDeg(posdir.heading, posdir.pitch, posdir.roll);

        // Compute the eyepoints orientation and position
        // wrt the earth centered frame - that is global coorinates
        SGQuatd ec2body = hlOr * hlToBody;

        // The cartesian position of the basic view coordinate
        SGVec3d position = SGVec3d::fromGeod(posdir.position);

        // This is rotates the x-forward, y-right, z-down coordinate system the where
        // simulation runs into the OpenGL camera system with x-right, y-up, z-back.
        SGQuatd q(-0.5, -0.5, 0.5, 0.5);
        
        posdir.position2 = position;
        posdir.direction2 = ec2body * q;
    }
};


struct SviewPos
{
    #if 0
    SviewPos(SGPropertyNode* node)
    {
        bool finished = false;
        m_name = node->getStringValue("name");
        simgear::PropertyList   children = node->getChildren("step");
        for (SGPropertyNode* child: children) {
            std::string type = child->getStringValue("type");
            std::shared_ptr<SviewStep> step;
            if (0) {}
            else if (type == "nearest-tower") {
                step.reset(new SviewStepNearestTower(child));
            }
            else if (type == "aircraft") {
                step.reset(new SviewStepAircraft(child));
            }
            else if (type == "move") {
                step.reset(new SviewStepMove(child));
            }
            else if (type == "rotate") {
                step.reset(new SviewStepRotate(child));
            }
            else if (type == "rotate-to-target") {
                step.reset(SviewStepRotateToTarget(child));
                finished = true;
            }
            else {
                SG_LOG(SG_GENERAL, SG_ALERT, "unrecognised step type: " << type);
                continue;
            }
            m_steps.push_back(step);
        }
        if (!finished) {
            std::unique_ptr<SviewStep> step = new SviewStepFinal;
        }
    }
    #endif
    
    void add_step(std::shared_ptr<SviewStep> step)
    {
        m_steps.push_back(step);
    }
    
    void add_step(SviewStep* step)
    {
        return add_step(std::shared_ptr<SviewStep>(step));
    }
    
    void evaluate(SviewPosDir& posdir)
    {
        for (auto step: m_steps) {
            step->evaluate(posdir);
        }
    };
    
    std::string m_name;
    std::vector<std::shared_ptr<SviewStep>>   m_steps;
};

static std::ostream& operator << (std::ostream& out, const osg::Vec3f& vec)
{
    return out << "Vec3f{"
            << " x=" << vec._v[0]
            << " y=" << vec._v[1]
            << " z=" << vec._v[2]
            << "}";
}

static std::ostream& operator << (std::ostream& out, const osg::Quat& quat)
{
    return out << "Quat{"
            << " x=" << quat._v[0]
            << " y=" << quat._v[1]
            << " z= " << quat._v[2]
            << " w=" << quat._v[3]
            << "}";
}



static std::ostream& operator << (std::ostream& out, const osg::Matrixd& matrix)
{
    osg::Vec3f  translation;
    osg::Quat   rotation;
    osg::Vec3f  scale;
    osg::Quat   so;
    matrix.decompose(translation, rotation, scale, so);
    return out << "Matrixd {"
            << " translation=" << translation
            << " rotation=" << rotation
            << " scale=" << scale
            << " so=" << so
            << "}";
}


struct SviewView
{
    /* Returns false if window has been closed. */
    virtual bool update(double dt) = 0;
    
    virtual ~SviewView() {}
};


/* A clone of current view. */
struct SviewViewClone : SviewView
{
    SviewViewClone(osgViewer::View* view)
    :
    m_view(view)
    {
        SG_LOG(SG_VIEW, SG_INFO, "m_view=" << m_view);
        SGPropertyNode* root;
        SGPropertyNode* sim;
        if (1) {
            /* User aircraft. */
            root = globals->get_props();
            sim = root->getNode("sim");
        }
        else {
            root = globals->get_props()->getNode("ai/models/multiplayer[]");
            sim = root->getNode("ai/models/multiplayer[]/set/sim");
        }
        int view_number_raw = sim->getIntValue("current-view/view-number-raw");
        SGPropertyNode* view_node = sim->getNode("view", view_number_raw);
        
        if (view_node->getBoolValue("config/eye-fixed")) {
            /* E.g. Tower view. */
            SG_LOG(SG_VIEW, SG_INFO, "eye-fixed");
            
            /* First move to centre of aircraft. */
            m_target.add_step(new SviewStepAircraft(root));
            m_target.add_step(new SviewStepMove(
                    -view_node->getDoubleValue("config/target-z-offset-m"),
                    -view_node->getDoubleValue("config/target-y-offset-m"),
                    -view_node->getDoubleValue("config/target-x-offset-m")
                    ));
            
            /* Set pitch and roll to zero, otherwise view from tower (as
            calculated by SviewStepTarget) rolls/pitches with aircraft. */
            m_target.add_step(new SviewStepDirectionMultiply(
                    1 /* heading */,
                    0 /* pitch */,
                    0 /* roll */
                    ));
            
            /* Current position is the target, so add a step that copies it to
            SviewPosDir.target. */
            m_target.add_step(new SviewStepCopyToTarget);
            
            /* Now set .m_eye up so that it looks from the nearest tower. */
            m_eye.add_step(new SviewStepNearestTower(sim));
            m_eye.add_step(new SviewStepTarget);
            
            /* At this point would like to move towards the target a little to
            avoid eye being inside the tower walls. But SviewStepTarget will
            have simply set posdir.direction2, so modifying posdir.heading etc
            will have no affect. Prob need to modify SviewStepTarget to set
            dirpos.heading,pitch,roll, then append a SviewStepFinal to convert
            to posdir.direction2. Alternatively, could we make all SviewStep*'s
            always work in terms of posdir.position2 and posdir.direction2,
            i.e. use SGQuatd for everything?

            [At some point it might be good to make this movement not change
            the height too.] */
            m_eye.add_step(new SviewStepMove(-30, 0, 0));
        }
        else {
            SG_LOG(SG_VIEW, SG_INFO, "not eye-fixed");
            /* E.g. Pilot view or Helicopter view. We assume eye position is
            relative to aircraft. */
            m_eye.add_step(new SviewStepAircraft(root));
            bool at_model = view_node->getBoolValue("config/at-model");
            SG_LOG(SG_VIEW, SG_INFO, "at_model=" << at_model);
            if (at_model) {
                /* E.g. Helicopter view. Move to centre of aircraft.
                
                config/target-z-offset-m seems to be +ve when moving backwards
                relative to the aircraft, so we need to negate the value we
                pass to SviewStepMove(). */
                m_eye.add_step(new SviewStepMove(
                        -view_node->getDoubleValue("config/target-z-offset-m"),
                        -view_node->getDoubleValue("config/target-y-offset-m"),
                        -view_node->getDoubleValue("config/target-x-offset-m")
                        ));
                
                /* Crudely preserve or don't preserve aircraft's heading,
                pitch and roll; this enables us to mimic Helicopter and Chase
                views. In theory we should evaluate the specified paths, but
                in practise we only need to multiply current values by 0 or 1.
                todo: add damping. */
                m_eye.add_step(new SviewStepDirectionMultiply(
                        view_node->getStringValue("config/eye-heading-deg-path")[0] ? 1 : 0,
                        view_node->getStringValue("config/eye-pitch-deg-path")[0] ? 1 : 0,
                        view_node->getStringValue("config/eye-roll-deg-path")[0] ? 1 : 0
                        ));
                /* Apply the current view rotation. */
                m_eye.add_step(new SviewStepRotate(
                        root->getDoubleValue("sim/current-view/heading-offset-deg"),
                        root->getDoubleValue("sim/current-view/pitch-offset-deg"),
                        root->getDoubleValue("sim/current-view/roll-offset-deg")
                        ));
            }
            else {
                /* E.g. pilot view. Move to the eye position.

                config/z-offset-m seems to be +ve when moving backwards
                relative to the aircraft, so we need to negate the value we
                pass to SviewStepMove(). */
                m_eye.add_step(new SviewStepMove(
                        -view_node->getDoubleValue("config/z-offset-m"),
                        -view_node->getDoubleValue("config/y-offset-m"),
                        -view_node->getDoubleValue("config/x-offset-m")
                        ));
                /* Apply the current view rotation. */
                m_eye.add_step(new SviewStepRotate(
                        root->getDoubleValue("sim/current-view/heading-offset-deg"),
                        -root->getDoubleValue("sim/current-view/pitch-offset-deg"),
                        root->getDoubleValue("sim/current-view/roll-offset-deg")
                        ));
            }
            if (at_model) {
                /* E.g. Helicopter view. Move eye away from aircraft.
                config/z-offset-m defaults to /sim/chase-distance-m (see
                fgdata:defaults.xml) which is -ve, e.g. -25m. */
                m_eye.add_step(new SviewStepMove(
                        view_node->getDoubleValue("config/z-offset-m"),
                        view_node->getDoubleValue("config/y-offset-m"),
                        view_node->getDoubleValue("config/x-offset-m")
                        ));
            }
            m_eye.add_step(new SviewStepFinal);
        }
    }
    
    bool update(double dt) override
    {
        bool valid = m_view->getCamera()->getGraphicsContext()->valid();
        if (valid) {
            SG_LOG(SG_VIEW, SG_BULK, "valid=" << valid);
            SviewPosDir posdir;
            m_target.evaluate(posdir);
            m_eye.evaluate(posdir);

            /* FGViewMgr::update(). */
            osg::Vec3d  position = toOsg(posdir.position2);
            osg::Quat   orientation = toOsg(posdir.direction2);

            /* CameraGroup::update() */
            osg::Matrix old_m = m_view->getCamera()->getViewMatrix();
            const osg::Matrix new_m(osg::Matrix::translate(-position)
                                         * osg::Matrix::rotate(orientation.inverse()));
            SG_LOG(SG_VIEW, SG_BULK, "old_m: " << old_m);
            SG_LOG(SG_VIEW, SG_BULK, "new_m: " << new_m);
            m_view->getCamera()->setViewMatrix(new_m);
        }
        return valid;
    }
    
    ~SviewViewClone()
    {
        osgViewer::ViewerBase* viewer_base = m_view->getViewerBase();
        auto composite_viewer = dynamic_cast<osgViewer::CompositeViewer*>(viewer_base);
        assert(composite_viewer);
        for (unsigned i=0; i<composite_viewer->getNumViews(); ++i) {
            osgViewer::View* view = composite_viewer->getView(i);
            SG_LOG(SG_VIEW, SG_ALERT, "composite_viewer view i=" << i << " view=" << view);
        }
        SG_LOG(SG_VIEW, SG_ALERT, "removing m_view=" << m_view);
        composite_viewer->stopThreading();
        composite_viewer->removeView(m_view);
        composite_viewer->startThreading();
    }
    
    private:
    
    SviewPos            m_eye;
    SviewPos            m_target;
    osgViewer::View*    m_view;
};


static std::vector<std::shared_ptr<SviewView>>  s_views;


void SviewAddClone(osgViewer::View* view)
{
    std::shared_ptr<SviewView> view_clone(new SviewViewClone(view));
    s_views.push_back(view_clone);
    
}

void SviewUpdate(double dt)
{
    for (size_t i=0; i<s_views.size(); /* inc in loop*/) {
        bool valid = s_views[i]->update(dt);
        if (valid) {
            i += 1;
        }
        else {
            SG_LOG(SG_VIEW, SG_INFO, "deleting SviewView i=" << i);
            s_views.erase(s_views.begin() + i);
        }
    }
}

void SviewClear()
{
    s_views.clear();
}