import os.path as osp
import open3d as o3d
import numpy as np
from torch.utils.data import Dataset
from torch import tensor
import pickle


# Class that represents the dataset
class Cars(Dataset):
    def __init__(self, path_pkl, view, transform=None):
        """Path to the dataset as well as the view are expected as an input. For `view` either Front or Back values
         are expected. Option Both is not implemented as it is assumed that the car can be seen only from one viewpoint"""
        assert view.casefold() == 'front' or view.casefold() == 'back', 'Wrong view selected. Only Front and Back views are available.'
        self.path_pkl = path_pkl
        self.view = 'FrontDataset.pkl' if 'front' in view else 'BackDataset.pkl'
        self.path_view = osp.join(self.path_pkl, self.view)
        self.transform = transform

    def __len__(self):
        # Just returns the length of the dataset
        with open(self.path_view, 'rb') as f:
            data = pickle.load(f)
        return len(data)

    def __getitem__(self, idx):
        # Returns and item at idx index from the dataset. Specifically, returns tuple (sample, label)
        with open(self.path_view, 'rb') as f:
            data = pickle.load(f)

        labels, samples = zip(*data)
        # Map stings to the numbers as torch.nn.CrossEntropyLoss (see Ego_learning.ipynb) requires label as a tensor
        labels_num = [0 if lab == "bmw" else
                      1 if lab == "vw" else
                      2 for lab in labels]

        # Create PointCloud object
        sample_pcl = o3d.geometry.PointCloud()
        # Assign numpy array values to that PointCloud object
        sample_pcl.points = o3d.utility.Vector3dVector(samples[idx])
        # Cast PointCloud to VoxelGrid.
        vox_grid = o3d.geometry.VoxelGrid.create_from_point_cloud(sample_pcl, voxel_size=0.15)  # voxel size is heuristically selected.
        # With voxels_size of 0.6, it is hard to recognize the car. But the smaller the voxel size, the bigger
        # matrix is required and the bigger is computational demand. To be optimized, if possible

        # Get the grid index. Those will be set to one. The rest will remain zero
        voxs = np.array([vox.grid_index for vox in vox_grid.get_voxels()])
        # Binary, because it contains only 0 or 1. Number size of vox_binary should be bigger than
        # the biggest dimension of voxs
        vox_binary = np.zeros((35, 35, 35), dtype=float)
        vox_binary[voxs] = 1
        return tensor(labels_num[idx]), np.expand_dims(vox_binary, axis=0)


# Dataset if NN expects to work directly with PCL data
class CarsPCL(Dataset):
    def __init__(self, path_pkl, view):
        assert view.casefold() == 'front' or view.casefold() == 'back', 'Wrong view selected. Only Front and Back views are available.'
        self.path_pkl = path_pkl
        self.view = 'FrontDataset_only_3.pkl' if 'front' in view else 'BackDataset_only_3.pkl'
        self.path_view = osp.join(self.path_pkl, self.view)
        with open(self.path_view, 'rb') as f:
            data = pickle.load(f)

        labels, _ = zip(*data)
        self.classes = labels

    def __len__(self):
        with open(self.path_view, 'rb') as f:
            data = pickle.load(f)
        return len(data)

    def __getitem__(self, idx):
        with open(self.path_view, 'rb') as f:
            data = pickle.load(f)
        labels, samples = zip(*data)
        labels_num = [0 if lab == "bmw" else
                      1 if lab == "vw" else
                      2 for lab in labels]
        shapes = [sample.shape for sample in samples]
        #choice = np.random.choice(len(samples[idx]), min(shapes)[0], replace=True)
        choice = np.random.choice(len(samples[idx]), 392, replace=True) # HARD CODED
        points_set = samples[idx][choice, :]
        return points_set, tensor([labels_num[idx]])

