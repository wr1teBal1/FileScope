# FileScope

# 一款简洁的文件资源管理器

'''mermaid

flowchart TD
    %% User Interface Layer
    subgraph "User Interface Layer"
        app["Files.App"]:::ui
        controls["Files.App.Controls"]:::ui
        views["Files.App.Views"]:::ui
        viewmodels["Files.App.ViewModels"]:::ui
        dialogs["Files.App.Dialogs"]:::ui
        userControls["Files.App.UserControls"]:::ui
    end

    %% Business Logic & Service Layer
    subgraph "Business Logic & Service Layer"
        actions["Files.App.Actions"]:::service
        data["Files.App.Data"]:::service
        services["Files.App.Services"]:::service
        storage["Files.App.Storage"]:::service
        shared["Files.Shared"]:::service
    end

    %% Native/Interop and Lower-Level Components
    subgraph "Native/Interop and Lower-Level Components"
        cswin32["Files.App.CsWin32"]:::interop
        launcher["Files.App.Launcher"]:::interop
        openDialog["Files.App.OpenDialog"]:::interop
        saveDialog["Files.App.SaveDialog"]:::interop
    end

    %% Background Tasks
    subgraph "Background Tasks"
        background["Files.App.BackgroundTasks"]:::background
    end

    %% Testing
    subgraph "Testing"
        uiTests["Files.App.UITests"]:::test
        interactionTests["Files.InteractionTests"]:::test
    end

    %% Interactions
    %% UI and MVVM binding
    views -- "bindsTo" --- viewmodels
    %% ViewModels call Services
    viewmodels -->|"calls"| services
    %% Services use Data and Storage components
    services -->|"uses"| data
    services -->|"accesses"| storage
    services -->|"utilizes"| shared
    %% Actions trigger service methods
    actions -->|"triggers"| services
    %% UI Controls and UserControls interaction
    controls -->|"renders"| views
    userControls -->|"updates"| viewmodels
    %% Native/Interop integration
    launcher -->|"initializes"| app
    cswin32 -->|"integrates"| viewmodels
    services -->|"invokes"| openDialog
    services -->|"invokes"| saveDialog
    %% Background tasks notify services
    background -->|"notifies"| services
    %% Testing interactions
    uiTests -->|"tests"| views
    interactionTests -->|"tests"| services

    %% Click Events for Component Mapping
    click app "https://github.com/files-community/files/blob/main/src/Files.App"
    click controls "https://github.com/files-community/files/blob/main/src/Files.App.Controls"
    click views "https://github.com/files-community/files/tree/main/src/Files.App/Views"
    click viewmodels "https://github.com/files-community/files/tree/main/src/Files.App/ViewModels"
    click dialogs "https://github.com/files-community/files/tree/main/src/Files.App/Dialogs"
    click userControls "https://github.com/files-community/files/tree/main/src/Files.App/UserControls"
    click actions "https://github.com/files-community/files/tree/main/src/Files.App/Actions"
    click data "https://github.com/files-community/files/tree/main/src/Files.App/Data"
    click services "https://github.com/files-community/files/tree/main/src/Files.App/Services"
    click storage "https://github.com/files-community/files/tree/main/src/Files.App/Storage"
    click shared "https://github.com/files-community/files/blob/main/src/Files.Shared"
    click cswin32 "https://github.com/files-community/files/blob/main/src/Files.App.CsWin32"
    click launcher "https://github.com/files-community/files/blob/main/src/Files.App.Launcher"
    click openDialog "https://github.com/files-community/files/blob/main/src/Files.App.OpenDialog"
    click saveDialog "https://github.com/files-community/files/blob/main/src/Files.App.SaveDialog"
    click background "https://github.com/files-community/files/blob/main/src/Files.App.BackgroundTasks"
    click uiTests "https://github.com/files-community/files/blob/main/tests/Files.App.UITests"
    click interactionTests "https://github.com/files-community/files/blob/main/tests/Files.InteractionTests"

    %% Styles
    classDef ui fill:#AED6F1,stroke:#1B4F72,stroke-width:2px;
    classDef service fill:#A9DFBF,stroke:#1D8348,stroke-width:2px;
    classDef interop fill:#F9E79F,stroke:#B7950B,stroke-width:2px;
    classDef background fill:#F5B7B1,stroke:#C0392B,stroke-width:2px;
    classDef test fill:#D2B4DE,stroke:#6C3483,stroke-width:2px;
    '''