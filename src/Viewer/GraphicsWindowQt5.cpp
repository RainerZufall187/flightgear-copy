// Copyright (C) 2017 James Turner
// derived from OSG GraphicsWindowQt by Wang Rui
//
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

#include <osg/DeleteHandler>
#include "GraphicsWindowQt5.hxx"
#include <osgViewer/ViewerBase>
#include <QInputEvent>

#include <QDebug>
#include <QThread>

#include <QScreen>
#include <QGuiApplication>
#include <QAbstractEventDispatcher>
#include <QOpenGLContext>
#include <QSurfaceFormat>

using namespace flightgear;

class QtKeyboardMap
{

public:
    QtKeyboardMap()
    {
        mKeyMap[Qt::Key_Escape     ] = osgGA::GUIEventAdapter::KEY_Escape;
        mKeyMap[Qt::Key_Delete   ] = osgGA::GUIEventAdapter::KEY_Delete;
        mKeyMap[Qt::Key_Home       ] = osgGA::GUIEventAdapter::KEY_Home;
        mKeyMap[Qt::Key_Enter      ] = osgGA::GUIEventAdapter::KEY_KP_Enter;
        mKeyMap[Qt::Key_End        ] = osgGA::GUIEventAdapter::KEY_End;
        mKeyMap[Qt::Key_Return     ] = osgGA::GUIEventAdapter::KEY_Return;
        mKeyMap[Qt::Key_PageUp     ] = osgGA::GUIEventAdapter::KEY_Page_Up;
        mKeyMap[Qt::Key_PageDown   ] = osgGA::GUIEventAdapter::KEY_Page_Down;
        mKeyMap[Qt::Key_Left       ] = osgGA::GUIEventAdapter::KEY_Left;
        mKeyMap[Qt::Key_Right      ] = osgGA::GUIEventAdapter::KEY_Right;
        mKeyMap[Qt::Key_Up         ] = osgGA::GUIEventAdapter::KEY_Up;
        mKeyMap[Qt::Key_Down       ] = osgGA::GUIEventAdapter::KEY_Down;
        mKeyMap[Qt::Key_Backspace  ] = osgGA::GUIEventAdapter::KEY_BackSpace;
        mKeyMap[Qt::Key_Tab        ] = osgGA::GUIEventAdapter::KEY_Tab;
        mKeyMap[Qt::Key_Space      ] = osgGA::GUIEventAdapter::KEY_Space;
        mKeyMap[Qt::Key_Delete     ] = osgGA::GUIEventAdapter::KEY_Delete;
        mKeyMap[Qt::Key_Alt      ] = osgGA::GUIEventAdapter::KEY_Alt_L;
        mKeyMap[Qt::Key_Shift    ] = osgGA::GUIEventAdapter::KEY_Shift_L;
        mKeyMap[Qt::Key_Control  ] = osgGA::GUIEventAdapter::KEY_Control_L;
        mKeyMap[Qt::Key_Meta     ] = osgGA::GUIEventAdapter::KEY_Meta_L;

        mKeyMap[Qt::Key_F1             ] = osgGA::GUIEventAdapter::KEY_F1;
        mKeyMap[Qt::Key_F2             ] = osgGA::GUIEventAdapter::KEY_F2;
        mKeyMap[Qt::Key_F3             ] = osgGA::GUIEventAdapter::KEY_F3;
        mKeyMap[Qt::Key_F4             ] = osgGA::GUIEventAdapter::KEY_F4;
        mKeyMap[Qt::Key_F5             ] = osgGA::GUIEventAdapter::KEY_F5;
        mKeyMap[Qt::Key_F6             ] = osgGA::GUIEventAdapter::KEY_F6;
        mKeyMap[Qt::Key_F7             ] = osgGA::GUIEventAdapter::KEY_F7;
        mKeyMap[Qt::Key_F8             ] = osgGA::GUIEventAdapter::KEY_F8;
        mKeyMap[Qt::Key_F9             ] = osgGA::GUIEventAdapter::KEY_F9;
        mKeyMap[Qt::Key_F10            ] = osgGA::GUIEventAdapter::KEY_F10;
        mKeyMap[Qt::Key_F11            ] = osgGA::GUIEventAdapter::KEY_F11;
        mKeyMap[Qt::Key_F12            ] = osgGA::GUIEventAdapter::KEY_F12;
        mKeyMap[Qt::Key_F13            ] = osgGA::GUIEventAdapter::KEY_F13;
        mKeyMap[Qt::Key_F14            ] = osgGA::GUIEventAdapter::KEY_F14;
        mKeyMap[Qt::Key_F15            ] = osgGA::GUIEventAdapter::KEY_F15;
        mKeyMap[Qt::Key_F16            ] = osgGA::GUIEventAdapter::KEY_F16;
        mKeyMap[Qt::Key_F17            ] = osgGA::GUIEventAdapter::KEY_F17;
        mKeyMap[Qt::Key_F18            ] = osgGA::GUIEventAdapter::KEY_F18;
        mKeyMap[Qt::Key_F19            ] = osgGA::GUIEventAdapter::KEY_F19;
        mKeyMap[Qt::Key_F20            ] = osgGA::GUIEventAdapter::KEY_F20;

        mKeyMap[Qt::Key_hyphen         ] = '-';
        mKeyMap[Qt::Key_Equal         ] = '=';

        mKeyMap[Qt::Key_division      ] = osgGA::GUIEventAdapter::KEY_KP_Divide;
        mKeyMap[Qt::Key_multiply      ] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
        mKeyMap[Qt::Key_Minus         ] = '-';
        mKeyMap[Qt::Key_Plus          ] = '+';
        //mKeyMap[Qt::Key_H              ] = osgGA::GUIEventAdapter::KEY_KP_Home;
        //mKeyMap[Qt::Key_                    ] = osgGA::GUIEventAdapter::KEY_KP_Up;
        //mKeyMap[92                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Up;
        //mKeyMap[86                    ] = osgGA::GUIEventAdapter::KEY_KP_Left;
        //mKeyMap[87                    ] = osgGA::GUIEventAdapter::KEY_KP_Begin;
        //mKeyMap[88                    ] = osgGA::GUIEventAdapter::KEY_KP_Right;
        //mKeyMap[83                    ] = osgGA::GUIEventAdapter::KEY_KP_End;
        //mKeyMap[84                    ] = osgGA::GUIEventAdapter::KEY_KP_Down;
        //mKeyMap[85                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Down;
        mKeyMap[Qt::Key_Insert        ] = osgGA::GUIEventAdapter::KEY_KP_Insert;
        //mKeyMap[Qt::Key_Delete        ] = osgGA::GUIEventAdapter::KEY_KP_Delete;
    }

    ~QtKeyboardMap()
    {
    }

    int remapKey(QKeyEvent* event)
    {
        KeyMap::iterator itr = mKeyMap.find(event->key());
        if (itr == mKeyMap.end())
        {
            return int(*(event->text().toLatin1().data()));
        }
        else
            return itr->second;
    }

    private:
    typedef std::map<unsigned int, int> KeyMap;
    KeyMap mKeyMap;
};

static QtKeyboardMap s_QtKeyboardMap;



GLWindow::GLWindow()
    : QWindow()
{
    _devicePixelRatio = 1.0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    connect(this, &QWindow::screenChanged, this, &GLWindow::onScreenChanged);
    onScreenChanged();
#endif
}

void GLWindow::onScreenChanged()
{
    qWarning() << Q_FUNC_INFO << "screen changed";
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    _devicePixelRatio = screen()->devicePixelRatio();
#endif
    syncGeometryWithOSG();
}

void GLWindow::syncGeometryWithOSG()
{
    const int w = width();
    const int h = height();
    
    int scaled_width = static_cast<int>(w *_devicePixelRatio);
    int scaled_height = static_cast<int>(h*_devicePixelRatio);
    
    if (_gw) {
        _gw->resized( x(), y(), scaled_width,  scaled_height);
        _gw->getEventQueue()->windowResize( x(), y(), scaled_width, scaled_height );
        _gw->requestRedraw();
        _gw->_updateContextNeeded = true;
        
        _gw->getEventQueue()->syncWindowRectangleWithGraphicsContext();
    }
}

GLWindow::~GLWindow()
{
    // close GraphicsWindowQt and remove the reference to us
    if( _gw )
    {
        _gw->close();
        _gw->_window = nullptr;
        _gw = nullptr;
    }
}

void GLWindow::processDeferredEvents()
{
    QQueue<QEvent::Type> deferredEventQueueCopy;
    {
        QMutexLocker lock(&_deferredEventQueueMutex);
        deferredEventQueueCopy = _deferredEventQueue;
        _eventCompressor.clear();
        _deferredEventQueue.clear();
    }

    while (!deferredEventQueueCopy.isEmpty())
    {
        QEvent event(deferredEventQueueCopy.dequeue());
        QWindow::event(&event);
    }
}

bool GLWindow::event( QEvent* event )
{
    // Reparenting GLWidget may create a new underlying window and a new GL context.
    // Qt will then call doneCurrent on the GL context about to be deleted. The thread
    // where old GL context was current has no longer current context to render to and
    // we cannot make new GL context current in this thread.

    // We workaround above problems by deferring execution of problematic event requests.
    // These events has to be enqueue and executed later in a main GUI thread (GUI operations
    // outside the main thread are not allowed) just before makeCurrent is called from the
    // right thread. The good place for doing that is right after swap in a swapBuffersImplementation.

    if (event->type() == QEvent::Hide)
    {
        // enqueue only the last of QEvent::Hide and QEvent::Show
        enqueueDeferredEvent(QEvent::Hide, QEvent::Show);
        return true;
    }
    else if (event->type() == QEvent::Show)
    {
        // enqueue only the last of QEvent::Show or QEvent::Hide
        enqueueDeferredEvent(QEvent::Show, QEvent::Hide);
        return true;
    }
    else if (event->type() == QEvent::ParentChange)
    {
        // enqueue only the last QEvent::ParentChange
        enqueueDeferredEvent(QEvent::ParentChange);
        return true;
    }
    else if (event->type() == QEvent::UpdateRequest)
    {
        osg::ref_ptr<osgViewer::ViewerBase> v;
        if (_gw->_viewer.lock(v)) {
          v->frame();
        }
    }

    // perform regular event handling
    return QWindow::event( event );
}

void GLWindow::setKeyboardModifiers( QInputEvent* event )
{
    int modkey = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);
    unsigned int mask = 0;
    if ( modkey & Qt::ShiftModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
    if ( modkey & Qt::ControlModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
    if ( modkey & Qt::AltModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_ALT;
    _gw->getEventQueue()->getCurrentEventState()->setModKeyMask( mask );
}

void GLWindow::resizeEvent( QResizeEvent* event )
{
    QWindow::resizeEvent(event);
    syncGeometryWithOSG();
}

void GLWindow::moveEvent( QMoveEvent* event )
{
    QWindow::moveEvent(event);
    syncGeometryWithOSG();
}

void GLWindow::keyPressEvent( QKeyEvent* event )
{
    setKeyboardModifiers( event );
    int value = s_QtKeyboardMap.remapKey( event );
    _gw->getEventQueue()->keyPress( value );

    // this passes the event to the regular Qt key event processing,
    // among others, it closes popup windows on ESC and forwards the event to the parent widgets
    if( _forwardKeyEvents )
        inherited::keyPressEvent( event );
}

void GLWindow::keyReleaseEvent( QKeyEvent* event )
{
    if( event->isAutoRepeat() )
    {
        event->ignore();
    }
    else
    {
        setKeyboardModifiers( event );
        int value = s_QtKeyboardMap.remapKey( event );
        _gw->getEventQueue()->keyRelease( value );
    }

    // this passes the event to the regular Qt key event processing,
    // among others, it closes popup windows on ESC and forwards the event to the parent widgets
    if( _forwardKeyEvents )
        inherited::keyReleaseEvent( event );
}

void GLWindow::mousePressEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseButtonPress( event->x()*_devicePixelRatio, event->y()*_devicePixelRatio, button );
}

void GLWindow::mouseReleaseEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseButtonRelease( event->x()*_devicePixelRatio, event->y()*_devicePixelRatio, button );
}

void GLWindow::mouseDoubleClickEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseDoubleButtonPress( event->x()*_devicePixelRatio, event->y()*_devicePixelRatio, button );
}

void GLWindow::mouseMoveEvent( QMouseEvent* event )
{
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseMotion( event->x()*_devicePixelRatio, event->y()*_devicePixelRatio );
}

void GLWindow::wheelEvent( QWheelEvent* event )
{
    setKeyboardModifiers( event );
    _gw->getEventQueue()->mouseScroll(
        event->orientation() == Qt::Vertical ?
            (event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN) :
            (event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_LEFT : osgGA::GUIEventAdapter::SCROLL_RIGHT) );
}

GraphicsWindowQt5::GraphicsWindowQt5(osg::GraphicsContext::Traits* traits)
:   _realized(false)
,   _updateContextNeeded(false)
{
    _traits = traits;
    init(0);
}

GraphicsWindowQt5::~GraphicsWindowQt5()
{
    OSG_INFO << "destroying GraphicsWindowQt5" << std::endl;
    close();

    // remove reference from GLWindow
    if ( _window )
        _window->_gw = nullptr;
}

bool GraphicsWindowQt5::init( Qt::WindowFlags f )
{
    // update _widget and parent by WindowData
    WindowData* windowData = _traits.get() ? dynamic_cast<WindowData*>(_traits->inheritedWindowData.get()) : 0;
    if ( !_window )
        _window = windowData ? windowData->_window : nullptr;

    // create widget if it does not exist
    _ownsWidget = (_window == NULL);
    if ( !_window )
    {
        // WindowFlags
        Qt::WindowFlags flags = f | Qt::Window | Qt::CustomizeWindowHint;
        if ( _traits->windowDecoration )
            flags |= Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;

        // create widget
        _window = new GLWindow();
        _window->setFlags(flags);
        _window->setSurfaceType(QSurface::OpenGLSurface);
        _window->setFormat(traits2qSurfaceFormat(_traits.get()));
        _window->create();
    }

    // set widget name and position
    // (do not set it when we inherited the widget)
    if ( _ownsWidget )
    {
        _window->setTitle( _traits->windowName.c_str() );
        _window->setPosition( _traits->x, _traits->y );
        QSize sz(_traits->width, _traits->height);
        if ( !_traits->supportsResize ) {
          _window->setMinimumSize( sz );
          _window->setMaximumSize( sz );
        } else {
          _window->resize( sz );
        }
    }

    _window->setGraphicsWindow( this );
    useCursor( _traits->useCursor );

    // initialize State
    setState( new osg::State );
    getState()->setGraphicsContext(this);

    // initialize contextID
    if ( _traits.valid() && _traits->sharedContext.valid() )
    {
        getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
        incrementContextIDUsageCount( getState()->getContextID() );
    }
    else
    {
        getState()->setContextID( osg::GraphicsContext::createNewContextID() );
    }

    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphicsContext();

    return true;
}

QSurfaceFormat GraphicsWindowQt5::traits2qSurfaceFormat( const osg::GraphicsContext::Traits* traits )
{
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);

    format.setAlphaBufferSize( traits->alpha );
    format.setRedBufferSize( traits->red );
    format.setGreenBufferSize( traits->green );
    format.setBlueBufferSize( traits->blue );
    format.setDepthBufferSize( traits->depth );
    format.setStencilBufferSize( traits->stencil );
  //  format.setSampleBuffers( traits->sampleBuffers );
    format.setSamples( traits->samples );

    format.setAlphaBufferSize( traits->alpha>0 );
    format.setDepthBufferSize( traits->depth );

    format.setSwapBehavior( traits->doubleBuffer ?
        QSurfaceFormat::DoubleBuffer :
        QSurfaceFormat::DefaultSwapBehavior);
    format.setSwapInterval( traits->vsync ? 1 : 0 );
    format.setStereo( traits->quadBufferStereo ? 1 : 0 );

    return format;
}

void GraphicsWindowQt5::qSurfaceFormat2traits( const QSurfaceFormat& format, osg::GraphicsContext::Traits* traits )
{
    traits->red = format.redBufferSize();
    traits->green = format.greenBufferSize();
    traits->blue = format.blueBufferSize();
    traits->alpha = format.alphaBufferSize();
    traits->depth = format.depthBufferSize();
    traits->stencil = format.stencilBufferSize();
    traits->samples = format.samples();

    traits->quadBufferStereo = format.stereo();
    traits->doubleBuffer = (format.swapBehavior() == QSurfaceFormat::DoubleBuffer);

    traits->vsync = format.swapInterval() >= 1;
}

osg::GraphicsContext::Traits* GraphicsWindowQt5::createTraits( const QWindow* window )
{
    osg::GraphicsContext::Traits *traits = new osg::GraphicsContext::Traits;

    qSurfaceFormat2traits( window->format(), traits );

    QRect r = window->geometry();
    traits->x = r.x();
    traits->y = r.y();
    traits->width = r.width();
    traits->height = r.height();

    traits->windowName = window->title().toLocal8Bit().data();
    Qt::WindowFlags f = window->flags();
    traits->windowDecoration = ( f & Qt::WindowTitleHint ) &&
                            ( f & Qt::WindowMinMaxButtonsHint ) &&
                            ( f & Qt::WindowSystemMenuHint );

    traits->supportsResize = true;

  /*
    QSizePolicy sp = window->sizePolicy();
    traits->supportsResize = sp.horizontalPolicy() != QSizePolicy::Fixed ||
                            sp.verticalPolicy() != QSizePolicy::Fixed;
*/
    return traits;
}

bool GraphicsWindowQt5::setWindowRectangleImplementation( int x, int y, int width, int height )
{
    if ( _window == NULL )
        return false;

    qDebug() << "setWRI window geometry to " << x << "," << y <<
      " w=" << width << " h=" << height;
    _window->setGeometry( x, y, width, height );
    return true;
}

void GraphicsWindowQt5::getWindowRectangle( int& x, int& y, int& width, int& height )
{
    if ( _window )
    {
        const QRect& geom = _window->geometry();
        x = geom.x();
        y = geom.y();
        width = geom.width();
        height = geom.height();
    }
}

bool GraphicsWindowQt5::setWindowDecorationImplementation( bool windowDecoration )
{
    Qt::WindowFlags flags = Qt::Window|Qt::CustomizeWindowHint;//|Qt::WindowStaysOnTopHint;
    if ( windowDecoration )
        flags |= Qt::WindowTitleHint|Qt::WindowMinMaxButtonsHint|Qt::WindowSystemMenuHint;
    _traits->windowDecoration = windowDecoration;

    if ( _window )
    {
        _window->setFlags( flags );
        return true;
    }

    return false;
}

bool GraphicsWindowQt5::getWindowDecoration() const
{
    return _traits->windowDecoration;
}

void GraphicsWindowQt5::grabFocus()
{
    if ( _window )
        _window->requestActivate();
}

void GraphicsWindowQt5::grabFocusIfPointerInWindow()
{
  #if 0
    if ( _widget->underMouse() )
        _widget->setFocus( Qt::ActiveWindowFocusReason );
    #endif
}

void GraphicsWindowQt5::raiseWindow()
{
    if ( _window )
        _window->raise();
}

void GraphicsWindowQt5::setWindowName( const std::string& name )
{
    if ( _window )
        _window->setTitle( QString::fromUtf8(name.c_str()) );
}

std::string GraphicsWindowQt5::getWindowName()
{
    return _window ? _window->title().toStdString() : "";
}

void GraphicsWindowQt5::useCursor( bool cursorOn )
{
    if ( _window )
    {
        _traits->useCursor = cursorOn;
        if ( !cursorOn ) _window->setCursor( Qt::BlankCursor );
        else _window->setCursor( _currentCursor );
    }
}

void GraphicsWindowQt5::setCursor( MouseCursor cursor )
{
    if ( cursor==InheritCursor && _window )
    {
        _window->unsetCursor();
    }

    switch ( cursor )
    {
    case NoCursor: _currentCursor = Qt::BlankCursor; break;
    case RightArrowCursor: case LeftArrowCursor: _currentCursor = Qt::ArrowCursor; break;
    case InfoCursor: _currentCursor = Qt::SizeAllCursor; break;
    case DestroyCursor: _currentCursor = Qt::ForbiddenCursor; break;
    case HelpCursor: _currentCursor = Qt::WhatsThisCursor; break;
    case CycleCursor: _currentCursor = Qt::ForbiddenCursor; break;
    case SprayCursor: _currentCursor = Qt::SizeAllCursor; break;
    case WaitCursor: _currentCursor = Qt::WaitCursor; break;
    case TextCursor: _currentCursor = Qt::IBeamCursor; break;
    case CrosshairCursor: _currentCursor = Qt::CrossCursor; break;
    case HandCursor: _currentCursor = Qt::OpenHandCursor; break;
    case UpDownCursor: _currentCursor = Qt::SizeVerCursor; break;
    case LeftRightCursor: _currentCursor = Qt::SizeHorCursor; break;
    case TopSideCursor: case BottomSideCursor: _currentCursor = Qt::UpArrowCursor; break;
    case LeftSideCursor: case RightSideCursor: _currentCursor = Qt::SizeHorCursor; break;
    case TopLeftCorner: _currentCursor = Qt::SizeBDiagCursor; break;
    case TopRightCorner: _currentCursor = Qt::SizeFDiagCursor; break;
    case BottomRightCorner: _currentCursor = Qt::SizeBDiagCursor; break;
    case BottomLeftCorner: _currentCursor = Qt::SizeFDiagCursor; break;
    default: break;
    };
    if ( _window ) _window->setCursor( _currentCursor );
}

bool GraphicsWindowQt5::valid() const
{
    return _window;
}

bool GraphicsWindowQt5::realizeImplementation()
{
    _window->show();

    // initialize GL context for the widget
    // defer this to makeCurrent which happens on the rendering thread

    _realized = true;

    // make sure the event queue has the correct window rectangle size and input range
    getEventQueue()->syncWindowRectangleWithGraphicsContext();
    return true;
}

bool GraphicsWindowQt5::isRealizedImplementation() const
{
    return _realized;
}

void GraphicsWindowQt5::closeImplementation()
{
    if ( _window )
        _window->close();

    delete _context;
    _realized = false;
}

void GraphicsWindowQt5::runOperations()
{
    // While in graphics thread this is last chance to do something useful before
    // graphics thread will execute its operations.
    if (_updateContextNeeded || (QOpenGLContext::currentContext() != _context)) {
        makeCurrent();
        _updateContextNeeded = false;
    }

    _window->beforeRendering();

    GraphicsWindow::runOperations();

    _window->afterRendering();
}

bool GraphicsWindowQt5::makeCurrentImplementation()
{
    if (!_context) {
        QOpenGLContext* shareContext = 0;
        if ( _traits->sharedContext.valid() ) {
          GraphicsWindowQt5* gw = dynamic_cast<GraphicsWindowQt5*>(_traits->sharedContext.get());
          if (gw) {
            shareContext = gw->_context;
            qDebug() << Q_FUNC_INFO << "have a share context";
          }
        }

        _context = new QOpenGLContext();
        _context->setShareContext(shareContext);
        _context->setFormat(_window->format());
        bool result = _context->create();
        if (!result)
        {
          OSG_WARN << "GraphicsWindowQt5: Can't create QOpenGLContext'" << std::endl;
          return false;
        }

        _context->makeCurrent(_window);
        // allow derived classes to do work now the context is initalised
        contextInitalised();
    }

    if (_context && (QThread::currentThread() != _context->thread())) {
      qWarning() << "attempt to make context current on wrong thread";
      return false;
    }

    _context->makeCurrent(_window);
    return true;
}

bool GraphicsWindowQt5::releaseContextImplementation()
{
    _context->doneCurrent();
    return true;
}

void GraphicsWindowQt5::swapBuffersImplementation()
{
    _context->swapBuffers(_window);
}

void GraphicsWindowQt5::requestWarpPointer( float x, float y )
{
    if ( _window )
        QCursor::setPos( _window->mapToGlobal(QPoint((int)x,(int)y)) );
}

bool GraphicsWindowQt5::checkEvents()
{
    // this gets run on the main thread
    if (_window->getNumDeferredEvents() > 0)
      _window->processDeferredEvents();

// todo - only if not running inside QApplication::exec; can we check this?

    QCoreApplication::processEvents(QEventLoop::AllEvents);

    return true;
}

void GraphicsWindowQt5::setViewer(osgViewer::ViewerBase* viewer)
{
     osg::ref_ptr<osgViewer::ViewerBase> previous(_viewer);
    _viewer = viewer;
    viewerChanged(previous.get());
}

void GraphicsWindowQt5::viewerChanged(osgViewer::ViewerBase*)
{
    // nothing
}

void GraphicsWindowQt5::requestRedraw()
{
  _window->requestUpdate();
  GraphicsWindow::requestRedraw();
}

void GraphicsWindowQt5::requestContinuousUpdate(bool needed)
{
  GraphicsWindow::requestContinuousUpdate(needed);
}

class Qt5WindowingSystem : public osg::GraphicsContext::WindowingSystemInterface
{
public:

    Qt5WindowingSystem()
    {
        OSG_INFO << "QtWindowingSystemInterface()" << std::endl;
    }

    ~Qt5WindowingSystem()
    {
        if (osg::Referenced::getDeleteHandler())
        {
            osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
            osg::Referenced::getDeleteHandler()->flushAll();
        }
    }

    // Access the Qt windowing system through this singleton class.
    static Qt5WindowingSystem* getInterface()
    {
        static Qt5WindowingSystem* qtInterface = new Qt5WindowingSystem;
        return qtInterface;
    }

    // Return the number of screens present in the system
    virtual unsigned int getNumScreens( const osg::GraphicsContext::ScreenIdentifier& /*si*/ )
    {
        return QGuiApplication::screens().size();
    }

    // Return the resolution of specified screen
    // (0,0) is returned if screen is unknown
    virtual void getScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si,
      osg::GraphicsContext::ScreenSettings& resolution )
    {
      QScreen* screen = qScreenFromSI(si);
      if (!screen) {
          qWarning() << Q_FUNC_INFO << "no screen for identifier";
          return;
      }

      resolution.width = screen->size().width();
      resolution.height = screen->size().height();
      resolution.colorDepth = screen->depth();
      resolution.refreshRate = screen->refreshRate();
    }

    // Set the resolution for given screen
    virtual bool setScreenSettings( const osg::GraphicsContext::ScreenIdentifier& /*si*/, const osg::GraphicsContext::ScreenSettings & /*resolution*/ )
    {
        OSG_WARN << "osgQt: setScreenSettings() not implemented yet." << std::endl;
        return false;
    }

    // Enumerates available resolutions
    virtual void enumerateScreenSettings( const osg::GraphicsContext::ScreenIdentifier& si,
      osg::GraphicsContext::ScreenSettingsList & resolutions )
    {
      QScreen* screen = qScreenFromSI(si);
      if (!screen) {
          qWarning() << Q_FUNC_INFO << "no screen for identifier";
          return;
      }

      resolutions.clear();
      osg::GraphicsContext::ScreenSettings ss;
      ss.width = screen->size().width();
      ss.height = screen->size().height();
      ss.colorDepth = screen->depth();
      ss.refreshRate = screen->refreshRate();
      resolutions.push_back(ss);
    }

    // Create a graphics context with given traits
    virtual osg::GraphicsContext* createGraphicsContext( osg::GraphicsContext::Traits* traits )
    {
        if (traits->pbuffer)
        {
            OSG_WARN << "osgQt: createGraphicsContext - pbuffer not implemented yet." << std::endl;
            return NULL;
        }
        else
        {
            osg::ref_ptr< GraphicsWindowQt5 > window = new GraphicsWindowQt5( traits );
            if (window->valid()) {
              return window.release();
            }
            else {
              qWarning() << "window is not valid";
              return NULL;
            }
        }
    }

private:
    QScreen* qScreenFromSI(const osg::GraphicsContext::ScreenIdentifier& si)
    {
      QList<QScreen*> screens = QGuiApplication::screens();
      if (screens.size() < si.screenNum)
        return screens.at(si.screenNum);

      return 0; // or should we return the primary screen?
    }

    // No implementation for these
    Qt5WindowingSystem( const Qt5WindowingSystem& );
    Qt5WindowingSystem& operator=( const Qt5WindowingSystem& );
};

namespace flightgear
{

void initQtWindowingSystem()
{
osg::GraphicsContext::setWindowingSystemInterface(Qt5WindowingSystem::getInterface());
}

} // of namespace flightgear