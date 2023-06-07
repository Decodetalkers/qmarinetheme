# About this Program

qt5ct has some bugs, on qt6 it will make QGuiApplications coredump, this made me very mad, before I thought it is the bug of qt, but after unset the platformtheme, it pass, so finially I see it maybe something happened with qt5ct, and I cannot find clues to fix it, so I try to write a new one to find the clues, after less the function, it will not coredump, but I do not use svn, so ...

and I don't like to use gui to set config, I like to use toml, so I try to write a new one, and want to try to use cpp20, even cpp23, and it is now what the Program is.. I want to write a platform theme since a long time, thanks to qt5ct, now I write a new one, thanks to qt5ct

so...it is just a training

It support qt5 and qt6, you need to write config

position is in `{XDG_CONFIG_PATh}/marinetheme5` or `{XDG_CONFIG_PATh}/marinetheme6`

example is like

```toml
theme = "Adwaita"
dialogtype = "xdgdesktopportal"
iconstyle = "breeze"
wheelscroll = 1
```

support kde , now you can use dolphin
