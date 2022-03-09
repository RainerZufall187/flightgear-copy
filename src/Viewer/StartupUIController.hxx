// StartupUIController.hxx - run UI on startup before main-loop
// Copyright (C) 2022 James Turner
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>

// forward decls
class StartupSystemAdapter;

class StartupUIController
{
public:
    StartupUIController();
    ~StartupUIController();
    
    void init();
    void shutdown();

    void createSubsystems();
    void createWindow();
    
    void frame();

    bool isDone() const;
private:
    friend class StartupSystemAdapter;
    class StartupUIControllerPrivate;

    std::unique_ptr<StartupUIControllerPrivate> d;
    
    void setNasalModules();
};

void runStartupUI();

