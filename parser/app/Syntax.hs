module Syntax where

data ASTNode = S Struct | I Interface
    deriving (Show)

--
--
-- Shared
--
--

data TypeMap = V String | KV String String
    deriving (Show)

--
--
-- Struct
--
--

type StructName = String

data Extends = Extends ExtendsName TypeMap
    deriving (Show)

type ExtendsName = String

type Implementation = Maybe [String]

type ConfigBlock = [ConfigOption]

data ConfigOption = ConfigOption ConfigKey ConfigValue
    deriving (Show)

type ConfigKey = String

data ConfigValue
    = Num Float
    | Str String
    deriving (Show)

data Struct = Struct StructName Extends Implementation ConfigBlock
    deriving (Show)

--
--
-- Interface
--
--

data Interface = Interface InterfaceName TypeMap NamedInterface
    deriving (Show)

type InterfaceName = String

type NamedInterface = String
